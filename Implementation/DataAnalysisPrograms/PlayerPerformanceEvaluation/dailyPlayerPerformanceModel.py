from ..loadData import load_race_df, add_features
import os, numpy as np, pandas as pd
from datetime import datetime, timezone
from dateutil.relativedelta import relativedelta
from dotenv import load_dotenv, find_dotenv
from sqlalchemy import create_engine, text

from sklearn.compose import ColumnTransformer
from sklearn.preprocessing import OneHotEncoder, StandardScaler
from sklearn.linear_model import LogisticRegression
from sklearn.pipeline import Pipeline

from ..loadData import load_race_df, add_features
from datetime import datetime, timezone
from dateutil.relativedelta import relativedelta
import numpy as np, pandas as pd, os, joblib

from sklearn.compose import ColumnTransformer
from sklearn.preprocessing import OneHotEncoder, StandardScaler
from sklearn.linear_model import LogisticRegression
from sklearn.pipeline import Pipeline
from sklearn.metrics import roc_auc_score
import joblib

load_dotenv(find_dotenv())
NUM_FEATURES = ["kdr", "dpg", "dtpg","playtime_s"]   
CAT_FEATURES = ["vehicle"]                           

MODEL_DIR = os.path.join(os.path.dirname(__file__), "models")
os.makedirs(MODEL_DIR, exist_ok=True)
MODEL_PATH = os.path.join(MODEL_DIR, "player_win_model.joblib")

def train_player_win_model(window_months=6, alpha_l2=1.0):
    # load data
    start = (datetime.now(timezone.utc) - relativedelta(months=window_months)).strftime("%Y-%m-%d")
    df = add_features(load_race_df(start=start))
    # target:win(1) or lose(0)
    y = (df["wins"] > df["losses"]).astype(int)

    X = df[NUM_FEATURES + CAT_FEATURES]

    pre = ColumnTransformer([
        ("num", StandardScaler(), NUM_FEATURES),
        ("cat", OneHotEncoder(handle_unknown="ignore", drop=None), CAT_FEATURES),
    ])
    clf = LogisticRegression(
        max_iter=1000, solver="lbfgs", C=1/alpha_l2, class_weight="balanced", n_jobs=None
    )
    pipe = Pipeline([("pre", pre), ("clf", clf)])


    # train model
    pipe.fit(X, y)
    p_train = pipe.predict_proba(X)[:, 1]
    auc = roc_auc_score(y, p_train)
    print(f"[train] AUC={auc:.3f}  n={len(y)}")

    p_ref = float(p_train.mean())  

    meta = {
        "trained_at": datetime.utcnow().isoformat(timespec="seconds")+"Z",
        "window_months": window_months,
        "num_features": NUM_FEATURES,
        "cat_features": CAT_FEATURES,
        "p_ref": p_ref,
        "auc": float(auc),
    }
    joblib.dump({"pipeline": pipe, "meta": meta}, MODEL_PATH)
    print(f"[ok] model saved → {MODEL_PATH}  p_ref={p_ref:.3f}")

def get_engine():
    url = (
        f"mysql+pymysql://{os.getenv('DB_USER')}:{os.getenv('DB_PASS')}"
        f"@{os.getenv('DB_HOST')}:{os.getenv('DB_PORT')}/{os.getenv('DB_NAME')}?charset=utf8mb4"
    )
    return create_engine(url, pool_pre_ping=True)

def evaluate_and_store_player_vehicle(window_months: int = 6, alpha: int = 10):
 
    # 1) load model and data
    obj = joblib.load(MODEL_PATH)
    pipe = obj["pipeline"]
    meta = obj["meta"]
    p_ref = float(meta["p_ref"])
    NUMS = meta["num_features"]
    CATS = meta["cat_features"]

    # 2) get past half year's data with features
    start = (datetime.now(timezone.utc) - relativedelta(months=window_months)).strftime("%Y-%m-%d")
    df = add_features(load_race_df(start=start))
    if df.empty:
        print("[warn] no data to evaluate")
        return

    # 3) predict win probabilities
    X = df[NUMS + CATS]
    p = pipe.predict_proba(X)[:, 1]
    df = df.assign(p_pred=p)

    # 4) calculate player-vehicle efficiency scores
    def agg_fn(g):
        n = len(g)
        p_bar = (g["p_pred"].sum() + alpha * p_ref) / (n + alpha)  # 经验贝叶斯
        score = 100.0 * (p_bar / p_ref)                            # 效率分（平均≈100）
        return pd.Series({"score": score, "n_matches": int(n)})

    uv = df.groupby(["user_id", "vehicle"], as_index=False).apply(agg_fn).reset_index(drop=True)
    if uv.empty:
        print("[warn] nothing aggregated")
        return

    uv["pct"] = uv.groupby("vehicle")["score"].rank(pct=True)

    # 5) insert into player_vehicle_percentiles
    snap = datetime.utcnow().date()
    window_str = f"{window_months*30}d"   # 例：6个月≈180天
    model_ver = "player_eff_v" + snap.strftime("%Y%m%d")

    rows = [{
        "d": snap, "w": window_str,
        "uid": str(r.user_id), "veh": str(r.vehicle),
        "s": float(r.score), "p": float(r.pct),
        "n": int(r.n_matches), "m": model_ver
    } for r in uv.itertuples(index=False)]

    eng = get_engine()
    with eng.begin() as conn:
        # create table if not exists
        conn.execute(text("""
        CREATE TABLE IF NOT EXISTS `player_vehicle_percentiles` (
          `snapshot_date` DATE NOT NULL,
          `window`        VARCHAR(16) NOT NULL,
          `user_id`       VARCHAR(64) NOT NULL,
          `vehicle`       VARCHAR(64) NOT NULL,
          `score`         DOUBLE NOT NULL,
          `pct`           DOUBLE NOT NULL,
          `n_matches`     INT NOT NULL,
          `model_ver`     VARCHAR(32) NOT NULL,
          PRIMARY KEY (`snapshot_date`,`window`,`user_id`,`vehicle`),
          KEY `ix_user` (`user_id`), KEY `ix_vehicle` (`vehicle`)
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
        """))

        conn.execute(text("""
            INSERT INTO `player_vehicle_percentiles`
              (`snapshot_date`,`window`,`user_id`,`vehicle`,`score`,`pct`,`n_matches`,`model_ver`)
            VALUES
              (:d,:w,:uid,:veh,:s,:p,:n,:m)
            ON DUPLICATE KEY UPDATE
              `score`=VALUES(`score`),
              `pct`=VALUES(`pct`),
              `n_matches`=VALUES(`n_matches`),
              `model_ver`=VALUES(`model_ver`);
        """), rows)

    eng.dispose()
    print(f"[OK] snapshot={snap}  wrote {len(rows)} rows to player_vehicle_percentiles")

if __name__ == "__main__":
    train_player_win_model(window_months=6, alpha_l2=1.0)
    evaluate_and_store_player_vehicle(window_months=6, alpha=10)
    