// 小工具：随机整数
const rand = (min, max) => Math.floor(Math.random() * (max - min + 1)) + min;

/* ---------- Bar 图 ---------- */
const vehicleLabels = ['APC', 'Tank', 'Jeep', 'Artillery', 'Bike', 'Drone'];
let vehicleCounts = [12, 25, 9, 6, 15, 4];

// 这里直接传 canvas 元素给 Chart，Chart.js v4 也完全支持
// new 这个chart对象，需要三个参数
//id
//{type:'bar',data:{labels,datasets}}
const barChart = new Chart(document.getElementById('barChart'), {
  type: 'bar',
  data: {
    labels: vehicleLabels,             // data.labels就是X轴的分类
    datasets: [{           //
      label: '使用次数',
      data: vehicleCounts,
      borderWidth: 1
      // 不指定颜色也可以，Chart.js 会自动配色
    }]
  },
  options: {
    responsive: true,
    maintainAspectRatio: false,        // ✅ 由外层 .chartbox 控制高度
    plugins: {
      title: { display: true, text: '各载具使用次数（示例）' },
      legend: { display: true }
    },
    interaction: { mode: 'nearest', intersect: true },
    scales: {
      x: { grid: { display: false } },
      y: { beginAtZero: true, ticks: { stepSize: 5 } }
    }
  }
});

document.getElementById('barUpdate').addEventListener('click', () => {
  barChart.data.datasets[0].data = vehicleLabels.map(() => rand(0, 30));
  barChart.update();
});

/* ---------- Line 图 ---------- */
const labels7  = ['Mon','Tue','Wed','Thu','Fri','Sat','Sun'];
const wins7    = [3, 5, 2, 6, 4, 7, 5];

const labels30 = Array.from({length:30}, (_,i) => `D${i+1}`);
const wins30   = labels30.map(() => rand(0, 8));

let using30 = false;

const lineChart = new Chart(document.getElementById('lineChart'), {
  type: 'line',
  data: {
    labels: labels7,
    datasets: [{
      label: '胜场',
      data: wins7,
      fill: false,
      tension: 0.2,
      pointRadius: 3
    }]
  },
  options: {
    responsive: true,
    maintainAspectRatio: false,
    plugins: {
      title: { display: true, text: '每日胜场趋势（示例）' },
      legend: { display: true },
      tooltip: { enabled: true }
    },
    scales: {
      x: { grid: { display: false }},
      y: { beginAtZero: true }
    }
  }
});

document.getElementById('lineToggle').addEventListener('click', () => {
  using30 = !using30;
  lineChart.data.labels = using30 ? labels30 : labels7;
  lineChart.data.datasets[0].data = using30 ? wins30 : wins7;
  lineChart.update();
});
