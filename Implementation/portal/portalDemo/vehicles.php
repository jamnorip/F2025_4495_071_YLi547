<?php
$page_title = "Vehicles";
include "includes/header.php";
include "includes/db.php";
?>

<section class="hero">
  <h1>Vehicle Statistics</h1>
</section>

<section class="filters">
  <div class="search-bar">
    <input type="text" id="search" placeholder="Search vehicles...">
  </div>

  <!-- Hardcoded country buttons (not dynamically generated) -->
  <div class="country-buttons">
    <button data-country="">All</button>
    <button data-country="USA">USA</button>
    <button data-country="Germany">Germany</button>
    <button data-country="UK">UK</button>
    <button data-country="France">France</button>
    <button data-country="China">China</button>
    <button data-country="USSR">USSR</button>
  </div>

  <!-- Hardcoded role buttons (not dynamically generated) -->
  <div class="role-buttons">
    <button data-role="">All roles</button>
    <button data-role="Main Battle Tank">Main Battle Tank</button>
    <button data-role="Medium Tank">Medium Tank</button>
    <button data-role="Light Tank">Light Tank</button>
    <button data-role="Heavy Tank">Heavy Tank</button>
  </div>
</section>

<section class="vehicle-table">
  <table>
    <thead>
      <tr>
        <th>Rank</th>
        <th>Name</th>
        <th>Country</th>
        <th>Role</th>
        <th>Matches Played</th>
        <th>Wins</th>
        <th>K/D Ratio</th>
        <th>Win Rate</th>
      </tr>
    </thead>
    <tbody id="vehicle-body">
      <!-- Initially empty, filled dynamically via AJAX -->
      <tr><td colspan="8" style="text-align:center;">Loading data...</td></tr>
    </tbody>
  </table>
</section>

<script>

// 表格体
const tbody = document.getElementById('vehicle-body');

// object that includes all input or click
let state = {
  q: '',
  country: '',
  role: ''
};

// 渲染表格行
function renderRows(rows) {
  tbody.innerHTML = '';
  if (rows.length === 0) {
    tbody.innerHTML = '<tr><td colspan="8" style="text-align:center;">No data found from database</td></tr>';
    return;
  }

  let rank = 1;
  rows.forEach(r => {
    const tr = document.createElement('tr');
    tr.innerHTML = `
      <td>${rank++}</td>
      <td>${r.Name}</td>
      <td>${r.Country}</td>
      <td>${r.Role}</td>
      <td>${r.MatchesPlayed}</td>
      <td>${r.Wins}</td>
      <td>${r.KDRatio}</td>
      <td>${r.WinRate}%</td>
    `;
    tbody.appendChild(tr);
  });
}

//main select function
async function fetchVehicles() {

  //to create a new url
  const params = new URLSearchParams({
    q: state.q,
    country: state.country,
    role: state.role
  });

  try {
    //send a request
    const res = await fetch(`vehicles_search.php?${params.toString()}`);
    const data = await res.json();
    renderRows(data.rows || []);
  } catch (err) {
    console.error('Fetch error:', err);
    tbody.innerHTML = '<tr><td colspan="8" style="text-align:center;color:red;">Failed to load data.</td></tr>';
  }
}

const searchInput = document.getElementById('search');
searchInput.addEventListener('input', () => {
  state.q = searchInput.value.trim();
  fetchVehicles(); 
});


// to add state as a country
document.querySelectorAll('.country-buttons button').forEach(btn => {
  btn.addEventListener('click', () => {
    state.country = btn.dataset.country || '';
    fetchVehicles();
  });
});

// to add state as a role
document.querySelectorAll('.role-buttons button').forEach(btn => {
  btn.addEventListener('click', () => {
    state.role = btn.dataset.role || '';
    fetchVehicles();
  });
});

// 首次加载
fetchVehicles();
</script>

<?php include "includes/footer.php"; ?>
