// ---------- Demo 数据（Line 图用） ----------
const labels7  = ['Mon','Tue','Wed','Thu','Fri','Sat','Sun'];
const wins7    = [3, 5, 2, 6, 4, 7, 5];

const labels30 = Array.from({length:30}, (_,i) => `D${i+1}`);
const wins30   = labels30.map(() => Math.floor(Math.random()*8));

let using30 = false; // 当前是否显示30天

// ---------- 工具 ----------
const $ = (id) => document.getElementById(id);
const setMetrics = (fMs, uMs) => { $('metrics').textContent = `fetch: ${fMs} ms | update: ${uMs} ms`; };
const setDetails = (txt) => { $('details').textContent = txt; };

// ---------- 初始化 Bar 图（载具使用） ----------
const barChart = new Chart($('barChart'), {
  type: 'bar',
  data: {
    labels: ['APC','Tank','Jeep','Artillery','Bike','Drone'],
    datasets: [{
      label: 'Used Times',
      data: [12, 25, 9, 6, 15, 4],
      borderWidth: 1
    }]
  },
  options: {
    responsive: true,
    maintainAspectRatio: false,
    plugins: {
      title: { display: true, text: 'Vehicle Usage (DB/Random)' },
      legend: { display: true },
      tooltip: { enabled: true } // onHover 默认有 tooltip
    },
    scales: {
      x: { grid: { display:false }},
      y: { beginAtZero: true, ticks: { stepSize: 5 } }
    },
    // onHover 示例：把鼠标下的柱子高亮（简单版）
    onHover(evt, activeEls) {
      const canvas = evt?.chart?.canvas;
      canvas.style.cursor = activeEls.length ? 'pointer' : 'default';
    },
    // onClick 示例：点柱子显示详情
    onClick(evt, activeEls) {
      if (!activeEls.length) return;
      const i = activeEls[0].index;
      const label = barChart.data.labels[i];
      const value = barChart.data.datasets[0].data[i];
      setDetails(`Bar clicked → ${label}: ${value}`);
      // 这里也可以触发：更新 Line 图/请求明细数据/打开侧栏……
    }
  }
});

// ---------- 初始化 Line 图（每日胜场） ----------
const lineChart = new Chart($('lineChart'), {
  type: 'line',
  data: {
    labels: labels7,
    datasets: [{
      label: 'Wins',
      data: wins7,
      tension: 0.2,
      pointRadius: 3,
      fill: false
    }]
  },
  options: {
    responsive: true,
    maintainAspectRatio: false,
    plugins: {
      title: { display:true, text:'Daily Wins (7d / 30d)' },
      legend: { display:true },
      tooltip: { enabled:true }
    },
    scales: {
      x: { grid: { display:false }},
      y: { beginAtZero:true }
    },
    onHover(evt, activeEls) {
      const canvas = evt?.chart?.canvas;
      canvas.style.cursor = activeEls.length ? 'pointer' : 'default';
    },
    onClick(evt, activeEls) {
      if (!activeEls.length) return;
      const i = activeEls[0].index;
      const label = lineChart.data.labels[i];
      const value = lineChart.data.datasets[0].data[i];
      setDetails(`Point clicked → ${label}: ${value}`);
    }
  }
});

// ---------- 动态刷新（从 PHP 获取数据 → 更新 Bar 图） ----------
$('btnRefresh').addEventListener('click', async () => {
  const t0 = performance.now();
  let json;
  try {
    const res = await fetch('getVehicles.php', { cache: 'no-store' });
    if (!res.ok) throw new Error(`HTTP ${res.status}`);
    json = await res.json();
  } catch (e) {
    setDetails('Fetch failed: ' + e.message);
    return;
  }
  const t1 = performance.now();

  // 更新数据（注意 labels 和 data 长度要一致）
  barChart.data.labels = json.labels;
  barChart.data.datasets[0].data = json.data;

  const u0 = performance.now();
  barChart.update();
  const u1 = performance.now();

  setMetrics(Math.round(t1 - t0), Math.round(u1 - u0));
  setDetails('Bar updated from server.');
});

// ---------- 切换 Line 图：7天 / 30天 ----------
$('btnToggleRange').addEventListener('click', () => {
  using30 = !using30;
  lineChart.data.labels = using30 ? labels30 : labels7;
  lineChart.data.datasets[0].data = using30 ? wins30 : wins7;
  lineChart.update();
  setDetails('Line toggled: ' + (using30 ? '30d' : '7d'));
});

// ---------- Vehicle 下拉过滤（简单演示） ----------
$('vehicleFilter').addEventListener('change', (e) => {
  const v = e.target.value;
  if (v === 'all') {
    // 还原（这里简单用初始数据，你也可以缓存原始数组）
    barChart.data.labels = ['APC','Tank','Jeep','Artillery','Bike','Drone'];
    barChart.data.datasets[0].data = [12, 25, 9, 6, 15, 4];
  } else {
    const allLabels = ['APC','Tank','Jeep','Artillery','Bike','Drone'];
    const allData   = [12, 25, 9, 6, 15, 4];
    const idx = allLabels.indexOf(v);
    if (idx >= 0) {
      barChart.data.labels = [v];
      barChart.data.datasets[0].data = [allData[idx]];
    }
  }
  barChart.update();
  setDetails('Filter: ' + v);
});
