import os
import numpy as np
import pandas as pd
from datetime import datetime, timezone
from dateutil.relativedelta import relativedelta

from sqlalchemy import create_engine, text
from sklearn.compose import ColumnTransformer
from sklearn.preprocessing import OneHotEncoder, StandardScaler
from sklearn.linear_model import LogisticRegression
from sklearn.pipeline import Pipeline
from sklearn.metrics import roc_auc_score
import joblib

from ..loadData import load_race_df, add_features


from dotenv import load_dotenv, find_dotenv
load_dotenv(find_dotenv())

def get_engine():
    url = (
        f"mysql+pymysql://{os.getenv('DB_USER')}:{os.getenv('DB_PASS')}"
        f"@{os.getenv('DB_HOST')}:{os.getenv('DB_PORT')}/{os.getenv('DB_NAME')}?charset=utf8mb4"
    )
    return create_engine(url, pool_pre_ping=True)

MODEL_DIR = os.path.join(os.path.dirname(__file__), "models")
os.makedirs(MODEL_DIR, exist_ok=True)
MODEL_PATH = os.path.join(MODEL_DIR, "vehicle_eff_model.joblib")

def compute_player_skill_eb(df: pd.DataFrame, alpha: int = 20) -> pd.DataFrame:


    win_flag = (df["wins"] > df["losses"]).astype(int)
    mu = win_flag.mean() if len(df) else 0.5

    g = df.groupby("user_id", as_index=False).agg(
        wins=("wins", "sum"),
        losses=("losses", "sum"),
        rounds=("rounds", "sum")
    )
    g["player_skill"] = (g["wins"] + alpha * mu) / (g["rounds"] + alpha)
    g["n_matches"] = g["rounds"]
    return g[["user_id", "player_skill", "n_matches"]]

def train_vehicle_efficiency(window_months: int = 6, alpha_skill: int = 20, min_matches_vehicle: int = 50):

    start = (datetime.now(timezone.utc) - relativedelta(months=window_months)).strftime("%Y-%m-%d")
    df = add_features(load_race_df(start=start))
    if df.empty:
        print("[warn] no data in window")
        return

    y = (df["wins"] > df["losses"]).astype(int)

    skill = compute_player_skill_eb(df, alpha=alpha_skill)
    d = df.merge(skill, on="user_id", how="left")
    d["player_skill"] = d["player_skill"].fillna(skill["player_skill"].median())

   
    X = d[["player_skill", "vehicle"]].copy()

    pre = ColumnTransformer([
        ("num", StandardScaler(), ["player_skill"]),
        ("veh", OneHotEncoder(handle_unknown="ignore", sparse_output=False), ["vehicle"])
    ])
    clf = LogisticRegression(max_iter=1000, solver="lbfgs", class_weight="balanced", C=1.0)

    pipe = Pipeline([("pre", pre), ("clf", clf)])
    pipe.fit(X, y)

    
    p_train = pipe.predict_proba(X)[:, 1]
    auc = roc_auc_score(y, p_train)
    p_ref = float(p_train.mean())  


    s_ref = float(d["player_skill"].mean())

   
    vehicles = sorted(d["vehicle"].unique().tolist())

    
    X_ref = pd.DataFrame({
        "player_skill": [s_ref] * len(vehicles),
        "vehicle": vehicles
    })
    p_vehicle = pipe.predict_proba(X_ref)[:, 1]  
    efficiency = 100.0 * (p_vehicle / p_ref)

    
    n_by_vehicle = d.groupby("vehicle")["rounds"].sum().reindex(vehicles).fillna(0).astype(int).to_list()

   
    rows = []
    for veh, pv, eff, n in zip(vehicles, p_vehicle, efficiency, n_by_vehicle):
        if n < min_matches_vehicle:
            
            continue
        rows.append({
            "vehicle": veh,
            "p_vehicle": float(pv),
            "efficiency": float(eff),
            "n_matches": int(n)
        })

    if not rows:
        print("[warn] no vehicles meet min_matches threshold")
        return

    # insert
    eng = get_engine()
    snap = datetime.utcnow().date()
    model_ver = "veh_v" + snap.strftime("%Y%m%d")
    with eng.begin() as conn:
        conn.execute(text("""
        CREATE TABLE IF NOT EXISTS vehicle_efficiency (
          snapshot_date DATE NOT NULL,
          window       VARCHAR(16) NOT NULL,
          vehicle      VARCHAR(64) NOT NULL,
          p_ref        DOUBLE NOT NULL,
          p_vehicle    DOUBLE NOT NULL,
          efficiency   DOUBLE NOT NULL,
          n_matches    INT NOT NULL,
          auc          DOUBLE NOT NULL,
          model_ver    VARCHAR(32) NOT NULL,
          PRIMARY KEY (snapshot_date, window, vehicle),
          KEY ix_vehicle (vehicle)
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
        """))

        # upsert
        conn.execute(
            text("""INSERT INTO vehicle_efficiency
                    (snapshot_date, window, vehicle, p_ref, p_vehicle, efficiency, n_matches, auc, model_ver)
                    VALUES (:d,:w,:veh,:pr,:pv,:eff,:n,:auc,:m)
                    ON DUPLICATE KEY UPDATE
                      p_ref=VALUES(p_ref), p_vehicle=VALUES(p_vehicle),
                      efficiency=VALUES(efficiency), n_matches=VALUES(n_matches),
                      auc=VALUES(auc), model_ver=VALUES(model_ver)"""),
            [{"d": snap, "w": f"{window_months*30}d", "veh": r["vehicle"],
              "pr": p_ref, "pv": r["p_vehicle"], "eff": r["efficiency"],
              "n": r["n_matches"], "auc": float(auc), "m": model_ver} for r in rows]
        )
    eng.dispose()

    # save model
    meta = {
        "trained_at": datetime.utcnow().isoformat(timespec="seconds")+"Z",
        "window_months": window_months,
        "alpha_skill": alpha_skill,
        "s_ref": s_ref,
        "p_ref": p_ref,
        "auc": float(auc),
        "min_matches_vehicle": min_matches_vehicle
    }
    joblib.dump({"pipeline": pipe, "meta": meta}, MODEL_PATH)
    print(f"[OK] {len(rows)} vehicles written. AUC={auc:.3f} p_ref={p_ref:.3f} model={MODEL_PATH}")

if __name__ == "__main__":
    train_vehicle_efficiency(window_months=6, alpha_skill=20, min_matches_vehicle=50)
