# read game log data from txt and store into database
import os, time, json, csv, hashlib
from datetime import datetime, timezone
from typing import List, Dict, Any

import pymysql
from pymysql.cursors import DictCursor
from dotenv import load_dotenv, find_dotenv

load_dotenv(find_dotenv())

# Configuration needs to be modified
LOG_DIR = os.getenv("LOG_DIR", "logs")
SLEEP_SECS = int(os.getenv("LOG_SCAN_INTERVAL", "10"))
FILE_READY_AGE = int(os.getenv("FILE_READY_AGE", "3"))
BATCH_SIZE = int(os.getenv("BATCH_SIZE", "1000"))

DB_HOST = os.getenv("DB_HOST", "127.0.0.1")
DB_PORT = int(os.getenv("DB_PORT", "3306"))
DB_USER = os.getenv("DB_USER", "root")
DB_PASS = os.getenv("DB_PASS", "")
DB_NAME = os.getenv("DB_NAME", "game")
DB_CHARSET = "utf8mb4"

# the log file columns is the same as the DB table `race`
RACE_COLS = [
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
    if x is None: return None
    s = str(x).strip()
    if not s: return None
    # ISO or "YYYY-MM-DD HH:MM:SS"
    try:
        return datetime.fromisoformat(s.replace("Z","+00:00")).astimezone(timezone.utc).replace(tzinfo=None)
    except Exception:
        try:
            return datetime.strptime(s, "%Y-%m-%d %H:%M:%S")
        except Exception:
            return None

def normalize_row(row: Dict[str, Any]) -> Dict[str, Any]:
    
    r = {k: (v.strip() if isinstance(v, str) else v) for k, v in row.items()}
    out = {
        "matchID":    r.get("matchID"),
        "userID":     r.get("userID"),
        "userName":   r.get("userName"),
        "vehicle":    r.get("vehicle"),
        "team":       r.get("team"),
        "startTime":  to_dt(r.get("startTime")),
        "endTime":    to_dt(r.get("endTime")),
        "playtime":   to_int(r.get("playtime")),
        "round":      to_int(r.get("round")),
        "win":        to_int(r.get("win")),
        "lose":       to_int(r.get("lose")),
        "kill":       to_int(r.get("kill")),
        "death":      to_int(r.get("death")),
        "damage":     to_int(r.get("damage")),
        "damageTake": to_int(r.get("damageTake")),
        "costUsed":   to_int(r.get("costUsed")),
    }
    # invalid record
    if not out["matchID"] or not out["userID"]:
        return {}
    # change negative to zero
    for k in ["playtime","round","win","lose","kill","death","damage","damageTake","costUsed"]:
        if out[k] is not None and out[k] < 0:
            out[k] = 0
    return out

def read_csv_records(path: str) -> List[Dict[str, Any]]:
    rows = []
    with open(path, "r", encoding="utf-8-sig", newline="") as f:
        rdr = csv.DictReader(f)
        headers = [h.strip() for h in rdr.fieldnames or []]
        need = set(RACE_COLS)
        miss = [c for c in need if c not in headers]
        if miss:
            raise ValueError(f"CSV缺少列: {miss} (文件: {path})")
        for row in rdr:
            rec = normalize_row(row)
            if rec:
                rows.append(rec)
    return rows

UPSERT_SQL = """
INSERT INTO `race`
(`matchID`,`userID`,`userName`,`vehicle`,`team`,`startTime`,`endTime`,
 `playtime`,`round`,`win`,`lose`,`kill`,`death`,`damage`,`damageTake`,`costUsed`)
VALUES
({vals})
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
VALUES_1ROW = "(" + ",".join(["%s"] * len(RACE_COLS)) + ")"

def insert_rows(conn, recs: List[Dict[str, Any]]) -> int:
    if not recs: return 0
    total = 0
    with conn.cursor() as cur:
        for i in range(0, len(recs), BATCH_SIZE):
            batch = recs[i:i+BATCH_SIZE]
            params = [tuple(batch[j][c] for c in RACE_COLS) for j in range(len(batch))]
            sql = UPSERT_SQL.replace("{vals}", ",".join([VALUES_1ROW]*len(batch)))
            cur.execute(sql, sum([list(p) for p in params], [])) 
            total += len(batch)
    conn.commit()
    return total

def scan_once():
    if not os.path.isdir(LOG_DIR):
        print(f"[warn] LOG_DIR not found: {LOG_DIR}")
        return
    csv_files = [os.path.join(LOG_DIR, n) for n in os.listdir(LOG_DIR)
                 if os.path.isfile(os.path.join(LOG_DIR, n)) and n.lower().endswith(".csv")]
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
                if already_ingested(conn, path, sha):
                    continue

                recs = read_csv_records(path)
                n = insert_rows(conn, recs) if recs else 0
                insert_manifest(conn, path, st.st_size,
                                datetime.utcfromtimestamp(st.st_mtime),
                                sha, rows=n, status="OK", message=None)
                print(f"[OK] {os.path.basename(path)} → {n} rows")
            except Exception as e:
                try:
                    st = os.stat(path)
                    sha = file_sha256(path)
                    insert_manifest(conn, path, st.st_size,
                                    datetime.utcfromtimestamp(st.st_mtime),
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