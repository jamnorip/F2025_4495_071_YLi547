from ..loadData import load_race_df, add_features
import os, joblib
import numpy as np
from datetime import datetime, timezone
from dateutil.relativedelta import relativedelta

MODEL_PATH = os.path.join(os.path.dirname(__file__), "models", "player_win_model.joblib")

def _load_model():
    obj = joblib.load(MODEL_PATH)
    return obj["pipeline"], obj["meta"]

def _efficiency_from_probs(p, p_ref, alpha=10):
   
    n = len(p)
    p_bar = (float(p.sum()) + alpha * p_ref) / (n + alpha)
    return 100.0 * (p_bar / p_ref), n

def player_efficiency(user_name: str, vehicle: str | None = None,
                      window_months: int = 6, alpha: int = 10):
    pipe, meta = _load_model()
    p_ref = meta["p_ref"]

    start = (datetime.now(timezone.utc) - relativedelta(months=window_months)).strftime("%Y-%m-%d")
    df = add_features(load_race_df(start=start))
    sub = df[df["user_name"] == user_name].copy()
    if vehicle:
        sub = sub[sub["vehicle"] == vehicle]

    if sub.empty:
        return {"ok": False, "error": "no_data"}

    NUM_FEATURES = meta["num_features"]
    CAT_FEATURES = meta["cat_features"]
    X = sub[NUM_FEATURES + CAT_FEATURES]

    p = pipe.predict_proba(X)[:, 1]         
    eff, n = _efficiency_from_probs(p, p_ref, alpha=alpha)

    return {
        "ok": True,
        "userName": user_name,
        "vehicle": vehicle,
        "window": f"{window_months}m",
        "n_matches": int(n),
        "efficiency": round(eff, 1),        
        "p_ref": round(p_ref, 3),
        "trained_at": meta["trained_at"],
        "auc": meta["auc"]
    }

if __name__ == "__main__":
    print(player_efficiency("Alice"))                    
    print(player_efficiency("Alice", vehicle="tank_a"))  