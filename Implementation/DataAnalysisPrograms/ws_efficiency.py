# -*- coding: utf-8 -*-

import json
import time
from functools import lru_cache
from typing import Optional

from fastapi import FastAPI, WebSocket, Query
from fastapi.responses import JSONResponse


from .PlayerPerformanceEvaluation.infer_efficiency import player_efficiency

app = FastAPI(title="Player Efficiency Service")

# =========================
# WebSocket：/ws/efficiency
# =========================
@app.websocket("/ws/efficiency")
async def ws_efficiency(websocket: WebSocket):
    await websocket.accept()
    while True:
        try:
            raw = await websocket.receive_text()
            req = json.loads(raw)
        except Exception:
            await websocket.send_text(json.dumps({"ok": False, "error": "bad_json"}))
            continue

        user = (req.get("userName") or "").strip()
        vehicle = (req.get("vehicle") or "").strip() or None
        window_months = int(req.get("windowMonths") or 6)  # months of data window, default 6
        alpha = int(req.get("alpha") or 10)                # EB smoothing strength

        if not user:
            await websocket.send_text(json.dumps({"ok": False, "error": "userName_required"}))
            continue

        try:
            out = _eff(user, vehicle, window_months, alpha)
        except FileNotFoundError:
            await websocket.send_text(json.dumps({
                "ok": False, "error": "model_not_found",
                "hint":"first run the daily training script to generate player_win_model.joblib"
            }))
            continue
        except Exception as e:
            await websocket.send_text(json.dumps({"ok": False, "error": "server_error", "detail": str(e)[:200]}))
            continue

        await websocket.send_text(json.dumps(out))

# =========================
# Optional：HTTP GET /api/efficiency
# =========================
@app.get("/api/efficiency")
def api_efficiency(
    userName: str = Query(..., description="UserName"),
    vehicle: Optional[str] = Query(None, description="vehicle"),
    windowMonths: int = Query(6, ge=1, le=24, description="months of data window, default 6"),
    alpha: int = Query(10, ge=0, le=100, description="EB smoothing strength, default 10")
):
    try:
        return _eff(userName.strip(), (vehicle or "").strip() or None, windowMonths, alpha)
    except FileNotFoundError:
        return JSONResponse(status_code=500, content={
            "ok": False, "error": "model_not_found",
            "hint": "first run the daily training script to generate player_win_model.joblib"
        })
    except Exception as e:
        return JSONResponse(status_code=500, content={"ok": False, "error": "server_error", "detail": str(e)[:200]})
