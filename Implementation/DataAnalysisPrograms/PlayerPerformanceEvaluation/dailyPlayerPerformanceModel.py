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

if __name__ == "__main__":
    train_player_win_model(window_months=6, alpha_l2=1.0)