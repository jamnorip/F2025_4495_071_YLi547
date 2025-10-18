const vehicleLabels = ['APC', 'Tank', 'Jeep', 'Artillery', 'Bike', 'Drone'];
let vehicleCounts1 = [12, 25, 9, 6, 15, 4];

const lineChart = new Chart(
  document.getElementById('lineChart'),
  {
    type: 'line',
    data: {
      labels: vehicleLabels,
      datasets: [
        {
          label: 'count of using',
          data: vehicleCounts1,
          fill: false,
          tension: 0.2, // 
          pointRadius: 3  // size of point
        }
      ]
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      plugins: {
        title: { display: true, text: 'legend' },
        legend: { display: true },
        tooltip: { enabled: true }   
      },
      scales: {
        x: { grid: { display: false } },
        y: { beginAtZero: true }
      }
    }
  }
);
