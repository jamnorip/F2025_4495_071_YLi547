let kdChart, winChart, dbChart, compareChart;
let playerStats = [];
let dbLabels = [];
let dbWinRateData = [];
const API_PLAYER_STATS = "http://localhost:3000/api/playerstats";

// ------------------------------
// 从 URL 读取 ?player= 参数
// ------------------------------
function getPlayerFromURL() {
  const p = new URLSearchParams(window.location.search).get("player");
  return p ? p.trim() : "";
}

// ------------------------------
// KD 趋势图
// ------------------------------
async function loadKDChart(player) {
  const p = encodeURIComponent(player);
  const res = await fetch(`getPlayerTrend.php?mode=Realistic&player=${p}`);
  const data = await res.json();
  const ctx = document.getElementById("kdChart");
  if (kdChart) kdChart.destroy();
  kdChart = new Chart(ctx, {
    type: "line",
    data: {
      labels: data.labels,
      datasets: [
        {
          label: `${data.player} - K/D Trend`,
          data: data.values,
          borderColor: "rgba(230,230,0,0.9)",
          backgroundColor: "rgba(230,230,0,0.25)",
          tension: 0.3,
          fill: true
        }
      ]
    },
    options: {
      maintainAspectRatio: false,
      plugins: { legend:false,
        labels: { color: "white" } },
      scales: {
        x: { ticks: { color: "white" }, grid: { color: "#333" } },
        y: { ticks: { color: "white" }, grid: { color: "#333" } }
      }
    }
  });
}

// ------------------------------
// 胜率图
// ------------------------------
async function loadWinChart(player) {
  const p = encodeURIComponent(player);
  const res = await fetch(`getWinRate.php?mode=Realistic&player=${p}`);
  const data = await res.json();
  const rows = Array.isArray(data) ? data : data.data || [];
  if (!rows.length) return;

  const colors = {
    "Main Battle Tank": "orange",
    "Heavy Tank": "red",
    "Medium Tank": "skyblue",
    "Light Tank": "green"
  };

  const ctx = document.getElementById("winChart");
  if (winChart) winChart.destroy();
  winChart = new Chart(ctx, {
    type: "bar",
    data: {
      labels: rows.map(r => r.mode),
      datasets: [
        {
          label: `${player} - Win Rate (%)`,
          data: rows.map(r => r.win_rate),
          backgroundColor: rows.map(
            r => colors[r.mode] || "rgba(200,200,200,0.6)"
          ),
          borderColor: "rgba(255,255,255,0.8)",
          borderWidth: 1
        }
      ]
    },
    options: {
      maintainAspectRatio: false,
      plugins: { legend:false,
        labels: { color: "white" } },
      scales: {
        x: { ticks: { color: "white" }, grid: { color: "#333" } },
        y: {
          ticks: { color: "white", callback: val => val + "%" },
          grid: { color: "#333" },
          beginAtZero: true,
          max: 100
        }
      }
    }
  });
}

// ------------------------------
// Top10 + 下拉
// ------------------------------
async function loadDBStatsOnce() {
  const res = await fetch(API_PLAYER_STATS);
  playerStats = await res.json();

  playerStats.sort((a, b) => b.WinRate - a.WinRate);
  const topPlayers = playerStats.slice(0, 10);

  const playerSelect = document.getElementById("playerSelect");
  const existing = Array.from(playerSelect.options).map(o => o.value);

  topPlayers.forEach(p => {
    if (!existing.includes(p.PlayerName)) {
      playerSelect.add(new Option(p.PlayerName, p.PlayerName));
    }
  });

  dbLabels = topPlayers.map(p => p.PlayerName);
  dbWinRateData = topPlayers.map(p => (p.WinRate * 100).toFixed(1));

  const ctx = document.getElementById("dbWinChart");
  if (dbChart) dbChart.destroy();
  dbChart = new Chart(ctx, {
    type: "bar",
    data: {
      labels: dbLabels,
      datasets: [
        {
          label: "Win Rate (%)",
          data: dbWinRateData,
          backgroundColor: dbLabels.map(() => "rgba(0,191,255,0.6)"),
          borderColor: "rgba(0,191,255,1)",
          borderWidth: 1
        }
      ]
    },
    options: {
      maintainAspectRatio: false,
      indexAxis: "y",
      plugins: { 
        legend: {
            display: false, 
            labels: { color: "white" }
        }},
      scales: {
        x: {
          ticks: { color: "white", callback: val => val + "%" },
          grid: { color: "#333" },
          beginAtZero: true,
          max: 100
        },
        y: { ticks: { color: "white" }, grid: { color: "#333" } }
      }
    }
  });

  updateSelectedPlayerCards();
}

// ------------------------------
// 卡片信息
// ------------------------------
function updateSelectedPlayerCards() {
  const player = document.getElementById("playerSelect").value;
  const found = playerStats.find(p => p.PlayerName === player);
  document.getElementById("winRateTitle").textContent = `${player} Win Rate`;
  document.getElementById("KDtitle").textContent = `${player} K/D`;
  document.getElementById("dbWinRate").textContent = found
    ? (found.WinRate * 100).toFixed(1)
    : "—";
  document.getElementById("dbKD").textContent =
    found && found.KD != null ? Number(found.KD).toFixed(2) : "—";

  if (dbChart) {
    const idx = dbLabels.indexOf(player);
    dbChart.data.datasets[0].backgroundColor = dbLabels.map((_, i) =>
      i === idx ? "yellow" : "rgba(0,191,255,0.6)"
    );
    dbChart.update();
  }
}

// ------------------------------
// Compare 区域逻辑
// ------------------------------
document.addEventListener("DOMContentLoaded", () => {
  const btn = document.getElementById("btnCompare");
  if (!btn) return;

  btn.addEventListener("click", async () => {
    const p1 = document.getElementById("comparePlayer1").value.trim();
    const p2 = document.getElementById("comparePlayer2").value.trim();
    const type = document.getElementById("compareType").value;
    const resultDiv = document.getElementById("compareResult");
    const ctx = document.getElementById("compareChart");

    if (!p1 || !p2) {
      alert("Please enter both player names.");
      return;
    }

    if (compareChart) compareChart.destroy();

    // ===============================
    // 模式 1：K/D Ratio Trend
    // ===============================
    if (type === "kd") {
      const res1 = await fetch(
        `getPlayerTrend.php?mode=Realistic&player=${encodeURIComponent(p1)}`
      );
      const res2 = await fetch(
        `getPlayerTrend.php?mode=Realistic&player=${encodeURIComponent(p2)}`
      );
      const data1 = await res1.json();
      const data2 = await res2.json();

      compareChart = new Chart(ctx, {
        type: "line",
        data: {
          labels: data1.labels,
          datasets: [
            {
              label: `${p1} - K/D`,
              data: data1.values,
              borderColor: "rgba(230,230,0,0.9)",
              fill: false
            },
            {
              label: `${p2} - K/D`,
              data: data2.values,
              borderColor: "rgba(0,191,255,0.9)",
              fill: false
            }
          ]
        },
        options: {
          responsive: true,
          plugins: { legend: { labels: { color: "white" } } },
          scales: {
            x: { ticks: { color: "white" }, grid: { color: "#444" } },
            y: { ticks: { color: "white" }, grid: { color: "#444" } }
          }
        }
      });
    }

    // ===============================
    // 模式 2：Win Rate by Vehicle Type
    // ===============================
    else if (type === "win") {
      const res1 = await fetch(
        `getWinRate.php?mode=Realistic&player=${encodeURIComponent(p1)}`
      );
      const res2 = await fetch(
        `getWinRate.php?mode=Realistic&player=${encodeURIComponent(p2)}`
      );
      const data1 = await res1.json();
      const data2 = await res2.json();

      const types = [
        "Main Battle Tank",
        "Heavy Tank",
        "Medium Tank",
        "Light Tank"
      ];
      const winMap1 = Object.fromEntries(data1.map(r => [r.mode, r.win_rate]));
      const winMap2 = Object.fromEntries(data2.map(r => [r.mode, r.win_rate]));

      compareChart = new Chart(ctx, {
        type: "bar",
        data: {
          labels: types,
          datasets: [
            {
              label: `${p1} Win Rate (%)`,
              data: types.map(t => winMap1[t] || 0),
              backgroundColor: "rgba(230,230,0,0.7)"
            },
            {
              label: `${p2} Win Rate (%)`,
              data: types.map(t => winMap2[t] || 0),
              backgroundColor: "rgba(0,191,255,0.7)"
            }
          ]
        },
        options: {
          responsive: true,
          plugins: { legend: { labels: { color: "white" } } },
          scales: {
            x: { ticks: { color: "white" }, grid: { color: "#444" } },
            y: {
              ticks: { color: "white", callback: val => val + "%" },
              grid: { color: "#444" },
              beginAtZero: true,
              max: 100
            }
          }
        }
      });
    }

    // 显示图表区
    resultDiv.style.display = "block";
  });

  // 当切换选项时清空旧图表
  const select = document.getElementById("compareType");
  if (select) {
    select.addEventListener("change", () => {
      const resultDiv = document.getElementById("compareResult");
      if (compareChart) {
        compareChart.destroy();
        compareChart = null;
      }
      resultDiv.style.display = "none";
    });
  }
});

// ------------------------------
// 初始化（不依赖 PHP/隐藏字段）
// ------------------------------
async function init() {
  const playerSelect = document.getElementById("playerSelect");
  if (!playerSelect) return;

  let currentPlayer = getPlayerFromURL();
  if (!currentPlayer) currentPlayer = playerSelect.value || "HaoSUO";

  if (![...playerSelect.options].some(o => o.value === currentPlayer)) {
    playerSelect.add(new Option(currentPlayer, currentPlayer));
  }
  playerSelect.value = currentPlayer;

  await loadDBStatsOnce();
  await loadKDChart(currentPlayer);
  await loadWinChart(currentPlayer);
  updateSelectedPlayerCards();

  playerSelect.addEventListener("change", async () => {
    const p = playerSelect.value;
    await loadKDChart(p);
    await loadWinChart(p);
    updateSelectedPlayerCards();
  });
}

window.addEventListener("load", init);
