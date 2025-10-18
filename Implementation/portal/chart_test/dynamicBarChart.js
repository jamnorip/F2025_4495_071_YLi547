fetch('getVehicles.php')    // This line runs the getVehicles.php script, and fetch sends a GET request to the backend
  .then(response => response.json())   // Convert the response to a JSON object
  .then(json => {
    const ctx = document.getElementById('barChart');
    new Chart(ctx, {
      type: 'bar',
      data: {
        labels: json.labels,    // "json" here is the whole response, and "labels" comes from it
        datasets: [{
          label: 'Usage Count',
          data: json.data,     // Use "data" from the JSON structure
          backgroundColor: 'rgba(54,162,235,0.5)',
          borderColor: 'rgba(54,162,235,1)',
          borderWidth: 1
        }]
      },
      options: {
        responsive: true,
        maintainAspectRatio: false,
        plugins: {
          title: { display: true, text: 'Vehicle Usage Count (from database)' },
          legend: { display: true }
        },
        scales: {
          x: { grid: { display: false } },
          y: { beginAtZero: true }
        }
      }
    });
  })
  .catch(err => console.error("Failed to load data:", err));

