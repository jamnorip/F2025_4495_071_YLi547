import os
import numpy as np
import pandas as pd
from sqlalchemy import create_engine, text
from dotenv import load_dotenv
from datetime import datetime, timedelta, timezone

load_dotenv()
def get_engine():
    url = (
        f"mysql+pymysql://{os.getenv('DB_USER')}:{os.getenv('DB_PASS')}"
        f"@{os.getenv('DB_HOST')}:{os.getenv('DB_PORT')}/{os.getenv('DB_NAME')}?charset=utf8mb4"
    )
    return create_engine(url, pool_pre_ping=True)

def load_race_df(start=None, end=None, limit=None):
    where = []
    params = {}
    if start:
        where.append("`startTime` >= :start")
        params["start"] = start
    if end:
        where.append("`endTime` <= :end")
        params["end"] = end
    where_sql = ("WHERE " + " AND ".join(where)) if where else ""

    limit_sql = f"LIMIT {int(limit)}" if limit else ""

    sql = text(f"""
        SELECT
          `matchID`      AS match_id,
          `userID`       AS user_id,
          `userName`     AS user_name,
          `vehicle`      AS vehicle,
          `team`         AS team,
          `startTime`    AS start_time,
          `endTime`      AS end_time,
          `playtime`     AS playtime_s,
          `round`        AS rounds,
          `win`          AS wins,
          `lose`         AS losses,
          `kill`         AS kills,
          `death`        AS deaths,
          `damage`       AS damage,
          `damageTake`   AS damage_taken,
          `costUsed`     AS cost_used
        FROM `race`
        {where_sql}
        {limit_sql}
    """)

    eng = get_engine()
    df = pd.read_sql(sql, eng, params=params, parse_dates=["start_time", "end_time"])
    eng.dispose()
    return df

def add_features(df: pd.DataFrame) -> pd.DataFrame:
    df = df.copy()

    
    df["duration_s"] = (df["end_time"] - df["start_time"]).dt.total_seconds().astype("float")

    
    deaths_safe = df["deaths"].replace(0, np.nan)
    rounds_safe = df["rounds"].replace(0, np.nan)

    df["kdr"]       = df["kills"] / deaths_safe                
    df["win_rate"]  = df["wins"]  / rounds_safe                  
    df["dpg"]       = df["damage"] / rounds_safe                 
    df["dtpg"]      = df["damage_taken"] / rounds_safe           
    df["accuracy"]  = np.nan                                     
    # df["cost_per_r"] = df["cost_used"] / rounds_safe

    
    df = df.drop_duplicates(subset=["match_id", "user_id"])      
    
    for col in ["kdr", "dpg", "dtpg", "cost_per_r"]:
        if col in df:
            df[col] = df[col].clip(lower=df[col].quantile(0.01), upper=df[col].quantile(0.99))

   
    df["kdr"]      = df["kdr"].fillna(0.0)
    df["win_rate"] = df["win_rate"].fillna(0.0)
    df["dpg"]      = df["dpg"].fillna(0.0)
    df["dtpg"]     = df["dtpg"].fillna(0.0)
    # df["cost_per_r"] = df["cost_per_r"].fillna(0.0)

    return df

if __name__ == "__main__":
    df_raw = load_race_df(start = (datetime.now(timezone.utc) - timedelta(days=180)).strftime("%Y-%m-%d"), end=None, limit=None)  
    df = add_features(df_raw)
    print(df.head())
    df.to_parquet("data/race_sample.parquet", index=False)
    
