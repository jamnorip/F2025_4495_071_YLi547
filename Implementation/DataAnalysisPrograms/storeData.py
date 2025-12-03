# read game log data from txt and store into database
import os, time, json, csv, hashlib
from datetime import datetime, timezone
from typing import List, Dict, Any

import pymysql
from pymysql.cursors import DictCursor
from pathlib import Path
# from dotenv import load_dotenv, find_dotenv

# load_dotenv(find_dotenv())

# Configuration needs to be modified
# LOG_DIR = "log"            
LOG_DIR = str((Path(__file__).resolve().parent / "log"))            
SLEEP_SECS = 10             
FILE_READY_AGE = 3          
BATCH_SIZE = 1000           
ALLOWED_EXTS = {'.csv', '.txt'}
# MySQL 
DB_HOST = "34.45.55.86"       
DB_PORT = 3306              
DB_USER = "chris"            
DB_PASS = "123456"                
DB_NAME = "worldrebalance"            
DB_CHARSET = "utf8mb4"      
# PlayerName, Vehicle, JoinTime, LeaveTime, PlayDuration, KillCount, DeathCount, IsWin, DamageDealt, DamageGet
# the log file columns is the same as the DB table `race`
RACE_COLS = [
    "PlayerName", "Vehicle", "JoinTime", "LeaveTime", "PlayDuration", "KillCount", "DeathCount", "IsWin", "DamageDealt", "DamageGet"
]
SRC_COLS = [
    "PlayerName","Vehicle","JoinTime","LeaveTime","PlayDuration",
    "KillCount","DeathCount","IsWin","DamageDealt","DamageGet"
]
DB_COLS = [
    "matchID","userID","userName","vehicle","team",
    "startTime","endTime","playtime","round","win","lose",
    "kill","death","damage","damageTake","costUsed"
]
def get_conn():
    return pymysql.connect(
        host=DB_HOST, port=DB_PORT, user=DB_USER, password=DB_PASS,
        database=DB_NAME, autocommit=False, charset=DB_CHARSET,
        cursorclass=DictCursor
    )

def ensure_manifest_table(conn):
    with conn.cursor() as cur:
        cur.execute("""
        CREATE TABLE IF NOT EXISTS `ingest_manifest` (
          `id` BIGINT PRIMARY KEY AUTO_INCREMENT,
          `file_path` VARCHAR(512) NOT NULL,
          `file_size` BIGINT NOT NULL,
          `file_mtime` DATETIME NOT NULL,
          `sha256` CHAR(64) NOT NULL,
          `rows_ingested` INT NOT NULL,
          `status` ENUM('OK','ERR') NOT NULL DEFAULT 'OK',
          `message` VARCHAR(512) NULL,
          `created_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
          UNIQUE KEY `ux_file` (`file_path`,`sha256`)
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
        """)
    conn.commit()

def already_ingested(conn, path, sha256_hex) -> bool:
    with conn.cursor() as cur:
        cur.execute("SELECT 1 FROM ingest_manifest WHERE file_path=%s AND sha256=%s LIMIT 1",
                    (path, sha256_hex))
        return cur.fetchone() is not None

def insert_manifest(conn, path, size, mtime_dt, sha256_hex, rows, status="OK", message=None):
    with conn.cursor() as cur:
        cur.execute("""INSERT INTO ingest_manifest
                       (file_path,file_size,file_mtime,sha256,rows_ingested,status,message)
                       VALUES (%s,%s,%s,%s,%s,%s,%s)""",
                    (path, size, mtime_dt, sha256_hex, rows, status, message))
    conn.commit()

def is_file_ready(path: str) -> bool:
    try:
        st = os.stat(path)
    except FileNotFoundError:
        return False
    return (time.time() - st.st_mtime) >= FILE_READY_AGE

def file_sha256(path: str, chunk: int = 1 << 20) -> str:
    h = hashlib.sha256()
    with open(path, "rb") as f:
        while True:
            b = f.read(chunk)
            if not b: break
            h.update(b)
    return h.hexdigest()

# datatype normalization
def to_int(x):
    try:
        return int(x) if x is not None and str(x).strip() != "" else None
    except Exception:
        return None

def to_dt(x):
    if x is None: 
        return None
    s = str(x).strip()
    if not s: 
        return None
    # ISO or "YYYY-MM-DD HH:MM:SS"
    try:
        return datetime.fromisoformat(s.replace("Z", "+00:00")).astimezone(timezone.utc).replace(tzinfo=None)
    except Exception:
            pass
        # Dec 2, 2025, 9:25:02 PM
    for fmt in ("%Y-%m-%d %H:%M:%S",
                "%b %d, %Y, %I:%M:%S %p",   # ← with comma
                "%b %d %Y %I:%M:%S %p"):    # ← without comma
        try:
            return datetime.strptime(s, fmt)
        except Exception:
            continue
    return None 

def normalize_row(row: Dict[str, Any]) -> Dict[str, Any]:
    win=0
    lose=0
    
    r = {k: (v.strip() if isinstance(v, str) else v) for k, v in row.items()}
    is_win = str(r.get("IsWin","")).strip().lower() in ("true","1","yes","y")
    win, lose = (1,0) if is_win else (0,1)
    out = {
        "matchID":    0,
        "userID":     r.get("PlayerName"),
        "userName":   r.get("PlayerName"),
        "vehicle":    r.get("Vehicle"),
        "team":       0,
        "startTime":  to_dt(r.get("JoinTime")),   
        "endTime":    to_dt(r.get("LeaveTime")),  
        "playtime":   to_seconds_duration(r.get("PlayDuration")),
        "round":      1,
        "win":        win,
        "lose":       lose,
        "kill":       to_int(r.get("KillCount")),
        "death":      to_int(r.get("DeathCount")),
        "damage":     to_int(r.get("DamageDealt")),
        "damageTake": to_int(r.get("DamageGet")),
        "costUsed":   0,
    }
    # # invalid record
    # if not out["matchID"] or not out["userID"]:
    #     return {}
    # change negative to zero
    for k in ["playtime","round","win","lose","kill","death","damage","damageTake","costUsed"]:
        if out[k] is not None and out[k] < 0:
            out[k] = 0
    return out

# def read_csv_records(path: str) -> List[Dict[str, Any]]:
#     rows = []
#     with open(path, "r", encoding="utf-8-sig", newline="") as f:
#         rdr = csv.DictReader(f, delimiter=';', skipinitialspace=True)
#         headers = [h.strip() for h in rdr.fieldnames or []]
#         need = set(RACE_COLS)
#         miss = [c for c in need if c not in headers]
#         if miss:
#             raise ValueError(f"CSV miss: {miss} (文件: {path})")
#         for row in rdr:
#             rec = normalize_row(row)
#             if rec:
#                 rows.append(rec)
#     return rows

def read_log_records(path: str) -> list[dict]:
    rows = []
    with open(path, "r", encoding="utf-8-sig", newline="") as f:
        rdr = csv.DictReader(f, delimiter=';', skipinitialspace=True)
        headers = [h.strip() for h in (rdr.fieldnames or [])]
        miss = [c for c in SRC_COLS if c not in headers]
        if miss:
            raise ValueError(f"缺少列: {miss} (文件: {path})")
        for row in rdr:
            row = {k.strip(): (v.strip() if isinstance(v, str) else v) for k, v in row.items()}
            rec = normalize_row(row)   
            if rec:
                rows.append(rec)
    return rows
# UPSERT_SQL = """
# INSERT INTO `race`
# (`matchID`,`userID`,`userName`,`vehicle`,`team`,`startTime`,`endTime`,
#  `playtime`,`round`,`win`,`lose`,`kill`,`death`,`damage`,`damageTake`,`costUsed`)
# VALUES
# ({vals})
# ON DUPLICATE KEY UPDATE
#  `userName`=VALUES(`userName`), `vehicle`=VALUES(`vehicle`), `team`=VALUES(`team`),
#  `startTime`=VALUES(`startTime`), `endTime`=VALUES(`endTime`),
#  `playtime`=VALUES(`playtime`), `round`=VALUES(`round`),
#  `win`=VALUES(`win`), `lose`=VALUES(`lose`),
#  `kill`=VALUES(`kill`), `death`=VALUES(`death`),
#  `damage`=VALUES(`damage`), `damageTake`=VALUES(`damageTake`),
#  `costUsed`=VALUES(`costUsed`);
# """

UPSERT_SQL = """
INSERT INTO `race`
(`matchID`,`userID`,`userName`,`vehicle`,`team`,`startTime`,`endTime`,
 `playtime`,`round`,`win`,`lose`,`kill`,`death`,`damage`,`damageTake`,`costUsed`)
VALUES
(%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s)
ON DUPLICATE KEY UPDATE
 `userName`=VALUES(`userName`), `vehicle`=VALUES(`vehicle`), `team`=VALUES(`team`),
 `startTime`=VALUES(`startTime`), `endTime`=VALUES(`endTime`),
 `playtime`=VALUES(`playtime`), `round`=VALUES(`round`),
 `win`=VALUES(`win`), `lose`=VALUES(`lose`),
 `kill`=VALUES(`kill`), `death`=VALUES(`death`),
 `damage`=VALUES(`damage`), `damageTake`=VALUES(`damageTake`),
 `costUsed`=VALUES(`costUsed`);
"""

# Generate multi-value placeholders (bulk inserts are faster)
VALUES_1ROW = "(" + ",".join(["%s"] * len(DB_COLS)) + ")"

def insert_rows(conn, recs: List[Dict[str, Any]]) -> int:
    if not recs:
        return 0
    total = 0
    with conn.cursor() as cur:
        for i in range(0, len(recs), BATCH_SIZE):
            batch = recs[i:i+BATCH_SIZE]
            params = [tuple(r.get(c) for c in DB_COLS) for r in batch]

            
            # print("DB_COLS len:", len(DB_COLS))
            # print("first tuple len:", len(params[0]))

            cur.executemany(UPSERT_SQL, params)
            total += len(batch)
    conn.commit()
    return total

def to_seconds_duration(s):
    if s is None or str(s).strip()=="":
        return None
    t = str(s).strip()
    if t.isdigit():
        return int(t)
    parts = t.split(":")
    try:
        if len(parts)==2:
            m, ss = int(parts[0]), int(parts[1]); return m*60 + ss
        if len(parts)==3:
            hh, m, ss = int(parts[0]), int(parts[1]), int(parts[2]); return hh*3600 + m*60 + ss
    except Exception:
        return None
    return None

def scan_once():
    if not os.path.isdir(LOG_DIR):
        print(f"[warn] LOG_DIR not found: {LOG_DIR}")
        return
    # csv_files = [os.path.join(LOG_DIR, n) for n in os.listdir(LOG_DIR)
    #              if os.path.isfile(os.path.join(LOG_DIR, n)) and n.lower().endswith(".csv")]
    csv_files = [
        os.path.join(LOG_DIR, n)
        for n in os.listdir(LOG_DIR)
        if os.path.isfile(os.path.join(LOG_DIR, n))
        and os.path.splitext(n)[1].lower() in ALLOWED_EXTS
    ]
    if not csv_files: return

    conn = get_conn()
    try:
        ensure_manifest_table(conn)
        for path in sorted(csv_files):
            try:
                if not is_file_ready(path):  
                    continue
                st = os.stat(path)
                sha = file_sha256(path)
                # skip already ingested files by sha256
                if already_ingested(conn, path, sha):
                    continue

                # recs = read_csv_records(path)
                recs = read_log_records(path)
                n = insert_rows(conn, recs) if recs else 0
                insert_manifest(conn, path, st.st_size,
                                datetime.fromtimestamp(st.st_mtime, timezone.utc),
                                sha, rows=n, status="OK", message=None)
                print(f"[OK] {os.path.basename(path)} → {n} rows")
            except Exception as e:
                try:
                    st = os.stat(path)
                    sha = file_sha256(path)
                    insert_manifest(conn, path, st.st_size,
                                    datetime.fromtimestamp(st.st_mtime, timezone.utc),
                                    sha, rows=0, status="ERR", message=str(e)[:500])
                except Exception:
                    pass
                print(f"[ERR] {os.path.basename(path)}: {e}")
    finally:
        conn.close()

def main():
    print(f"[watch] {os.path.abspath(LOG_DIR)}  every {SLEEP_SECS}s")
    while True:
        scan_once()
        time.sleep(SLEEP_SECS)

if __name__ == "__main__":
    main()