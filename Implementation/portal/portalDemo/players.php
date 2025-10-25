<?php
$page_title = "Players";
include __DIR__ . "/includes/header.php";
?>
<link rel="stylesheet" href="assets/css/player-style.css">

<section class="hero">
  <h1>Player Performance Overview</h1>
  <p>Statistics and trends for players in <b>Realistic Battles</b>.</p>
</section>

<!-- Dashboard -->
<section class="player-section">
  <h2>Player Statistics Dashboard</h2>
  <p>Select a player to view their K/D trend and win rate performance.</p>


  <!-- from database , select frame -->
  <div class="player-select">
    <select id="playerSelect">
      <option value="HaoSUO">HaoSUO</option>
      <option value="NoobMaster">NoobMaster</option>
      <option value="AcePilot">AcePilot</option>
    </select>
  </div>

  <!-- Charts -->
  <div class="chart-container">
    <div class="chart-box">
      <h3>K/D Ratio Trend (Last 30 Days)</h3>
      <p>K/D (Kill/Death) shows how many opponents a player defeats before dying once.</p>
      <canvas id="kdChart"></canvas>
    </div>

    <div class="chart-box">
      <h3>Win Rate by Vehicle Type</h3>
      <p>Comparison of win rate across Main Battle, Heavy, Medium, and Light tanks.</p>
      <canvas id="winChart"></canvas>
    </div>
  </div>
</section>

<hr class="section-divider">

<!-- Real-Time Stats -->
<section class="player-section">
  <h2 class="section-title-blue">Real-Time Player Statistics</h2>
  <p>Updated summary of player performance based on recent battle records.</p>

  <!-- Summary Cards -->
  <div class="realtime-cards">
    <div class="realtime-card">
      <h3 id="winRateTitle">Player Win Rate</h3>
      <p><span id="dbWinRate">—</span>%</p>
    </div>

    <div class="realtime-card kd">
      <h3 id="KDtitle">Player K/D</h3>
      <p><span id="dbKD">—</span></p>
    </div>
  </div>

  <!-- Win Rate Chart -->
  <div class="db-chart-section">
    <h3>Top 10 Players by Win Rate</h3>
    <p>Comparison of all players’ win rate.</p>
    <canvas id="dbWinChart"></canvas>
  </div>
</section>

<hr class="section-divider">

<?php include "includes/footer.php"; ?>

<!-- js import -->
<script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
<script src="assets/js/player.js" defer></script>



