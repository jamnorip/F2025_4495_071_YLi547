let kdChart, winChart, dbChart;
let playerStats = [];
let dbLabels = [];
let dbWinRateData = [];
const API_PLAYER_STATS = 'http://localhost:3000/api/playerstats';

// [
//   {
//     "PlayerName": "NoobMaster",
//     "TotalMatches": 18,
//     "Wins": "12",
//     "WinRate": "0.67",
//     "TotalKills": "61",
//     "TotalDeaths": "42",
//     "KD": "1.45"
//   },
//   {


// Load KD trend chart
async function loadKDChart(player) {
    const res = await fetch(`getPlayerTrend.php?mode=Realistic&player=${player}`); //mock analyzed data from getPlayerTrend
    const data = await res.json();
    const ctx = document.getElementById('kdChart');
    if (kdChart) kdChart.destroy();
    kdChart = new Chart(ctx, {
        type: 'line',
        data: {
            labels: data.labels,
            datasets: [{
                label: `${data.player} - K/D Trend`,
                data: data.values,
                borderColor: 'rgba(230,230,0,0.9)',
                backgroundColor: 'rgba(230,230,0,0.25)',
                tension: 0.3,
                fill: true,
            }]
        },
        options: {
            maintainAspectRatio: false,
            plugins: {
                legend: {
                    labels: {
                        color: 'white'
                    }
                }
            },
            scales: {
                x: {
                    ticks: {
                        color: 'white'
                    },
                    grid: {
                        color: '#333'
                    }
                },
                y: {
                    ticks: {
                        color: 'white'
                    },
                    grid: {
                        color: '#333'
                    }
                }
            }
        }
    });
}

// Load Win Rate chart
async function loadWinChart(player) {
    try {
        const res = await fetch(`getWinRate.php?mode=Realistic&player=${player}`); // to mock analyzed data from getWinRate.php
        const data = await res.json();
        const rows = Array.isArray(data) ? data : (data.data || []);
        if (!rows.length) return;

        const colors = {
            "Main Battle Tank": 'orange',
            "Heavy Tank": 'red',
            "Medium Tank": 'skyblue',
            "Light Tank": 'green'
        };

        const ctx = document.getElementById('winChart');
        if (winChart) winChart.destroy();

        winChart = new Chart(ctx, {
            type: 'bar',
            data: {
                labels: rows.map(r => r.mode),
                datasets: [{
                    label: `${player} - Win Rate (%)`,
                    data: rows.map(r => r.win_rate),
                    backgroundColor: rows.map(r => colors[r.mode] || 'rgba(200,200,200,0.6)'),
                    borderColor: 'rgba(255,255,255,0.8)',
                    borderWidth: 1
                }]
            },
            options: {
                maintainAspectRatio: false,
                plugins: {
                    legend: {
                        labels: {
                            color: 'white'
                        }
                    }
                },
                scales: {
                    x: {
                        ticks: {
                            color: 'white'
                        },
                        grid: {
                            color: '#333'
                        }
                    },
                    y: {
                        ticks: {
                            color: 'white',
                            callback: val => val + '%'
                        },
                        grid: {
                            color: '#333'
                        },
                        beginAtZero: true,
                        max: 100
                    }
                }
            }
        });
    } catch (err) {
        console.error(' loadWinChart error:', err);
    }
}

// Load DB stats (Top10)
async function loadDBStatsOnce() {
    try {
        //to get the play_stats[]
        const res = await fetch(API_PLAYER_STATS);
        //convert to json
        playerStats = await res.json();

        //sort win rate!!!
        playerStats.sort((a, b) => b.WinRate - a.WinRate);
        const topPlayers = playerStats.slice(0, 10);

        const playerSelect = document.getElementById('playerSelect');
        const existing = Array.from(playerSelect.options).map(o => o.value);


        //loop
        topPlayers.forEach(p => {
            if (!existing.includes(p.PlayerName)) {
                //create dropdown menu options
                const opt = document.createElement('option');
                opt.value = p.PlayerName;
                opt.textContent = p.PlayerName;
                //add each option
                playerSelect.appendChild(opt);
            }
        });

        //to get chart's labels
        dbLabels = topPlayers.map(p => p.PlayerName);
        dbWinRateData = topPlayers.map(p => (p.WinRate * 100).toFixed(1));

        const ctx = document.getElementById('dbWinChart');
        if (dbChart) dbChart.destroy(); //!!!!
        dbChart = new Chart(ctx, {
            type: 'bar',
            data: {
                labels: dbLabels,
                datasets: [{
                    label: 'Win Rate (%)',
                    data: dbWinRateData,
                    backgroundColor: dbLabels.map(() => 'rgba(0,191,255,0.6)'),
                    borderColor: 'rgba(0,191,255,1)',
                    borderWidth: 1
                }]
            },
            options: {
                maintainAspectRatio: false,
                indexAxis: 'y',
                plugins: {
                    legend: {
                        labels: {
                            color: 'white'
                        }
                    },
                    // title: {
                    //   display: true,
                    //   // // text: 'Top 10 Players by Win Rate',
                    //   // color: 'yellow',
                    //   // font: { size: 16 }
                    // }
                },
                scales: {
                    x: {
                        ticks: {
                            color: 'white',
                            callback: val => val + '%'
                        },
                        grid: {
                            color: '#333'
                        },
                        beginAtZero: true,
                        max: 100
                    },
                    y: {
                        ticks: {
                            color: 'white'
                        },
                        grid: {
                            color: '#333'
                        }
                    }
                }
            }
        });

        updateSelectedPlayerCards();
    } catch (e) {
        console.error('DB stats error:', e);
    }
}

function updateSelectedPlayerCards() {
    const player = document.getElementById('playerSelect').value;
    const found = playerStats.find(p => p.PlayerName === player);
    document.getElementById('winRateTitle').textContent = `${player} Win Rate`;
    document.getElementById('KDtitle').textContent = `${player} K/D`;
    document.getElementById('dbWinRate').textContent = found ? (found.WinRate * 100).toFixed(1) : '—';
    document.getElementById('dbKD').textContent = found && found.KD != null ?
        Number(found.KD).toFixed(2) :
        '—';

    // highlight
    if (dbChart) {
        //find index
        const idx = dbLabels.indexOf(player);
        dbChart.data.datasets[0].backgroundColor = dbLabels.map((_, i) => i === idx ? 'yellow' : 'rgba(0,191,255,0.6)');
        dbChart.update();
    }
}

// //
function init() {
    const playerSelect = document.getElementById('playerSelect');

    function reloadCharts() {
        const player = playerSelect.value;
        loadKDChart(player);
        loadWinChart(player);
    }
    reloadCharts();
    loadDBStatsOnce();

    //to link three chart
    playerSelect.addEventListener('change', () => {
        reloadCharts();
        updateSelectedPlayerCards();
    });
}
init();
