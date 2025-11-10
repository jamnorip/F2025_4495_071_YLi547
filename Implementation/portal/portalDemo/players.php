<?php
$page_title = "Players";
include "includes/header.php";

//判断权限
require_once 'includes/LoginManager.php';
$isLoginOrNot = LoginManager::verifyLogin();
//判断权限完，然后在前端js里面放入这个值，通知js前端，然后一旦登录，所有的逻辑都在js里面所以要把这个登录是否的结果给前端js

$selectedPlayer = trim($_GET['player'] ?? 'HaoSUO'); // 默认展示 HaoSUO
?>
<link rel="stylesheet" href="assets/css/style.css">


<!-- <section class="hero">
  <h1>Player Performance Overview</h1>
  <p>Statistics and trends for players in <b>Realistic Battles</b>.</p>
</section> -->

<section class="player-section">
  <h2>Player Statistics Dashboard</h2>
  <p>Select a player to view their K/D trend and win rate performance.</p>
  <div class="player-select">
    <select id="playerSelect">
      <option value="HaoSUO">HaoSUO</option>
      <option value="NoobMaster">NoobMaster</option>
      <option value="AcePilot">AcePilot</option>
    </select>
    <input type="hidden" id="defaultPlayer" value="<?php echo $_GET['player'] ?? 'HaoSUO'; ?>">
  </div>
  <h2 class="section-title-blue">Real-Time Player Statistics</h2>
  <div class="realtime-cards">
    <div class="realtime-card kd">
      <h3 id="KDtitle">Player K/D</h3>
      <p><span id="dbKD">—</span></p>
    </div>
    <div class="realtime-card">
      <h3 id="winRateTitle">Player Win Rate</h3>
      <p><span id="dbWinRate">—</span>%</p>
    </div>
    
  </div>

  <!-- <hr class="section-divider"> -->
  
</section>

<section class="player-section">
  

  <div class="chart-container">
    <div class="chart-box">
      <h3>K/D Ratio Trend (Last 30 Days)</h3>
      <canvas id="kdChart"></canvas>
    </div>
    <div class="chart-box">
      <h3>Win Rate by Vehicle Type</h3>
      <canvas id="winChart"></canvas>
    </div>
  </div>

  <div class="db-chart-section">
    <h3>Top 10 Players by Win Rate</h3>
    <canvas id="dbWinChart"></canvas>
  </div>
</section>



<hr class="section-divider">

<!-- compare-section -->
<?php if ($isLoginOrNot): ?>
<section id="compare-section" style="margin: 50px auto;">
  <h2>Compare Players</h2>
  <p>Enter two player names and choose what to compare.</p>

  <div class="compare-inputs">
    <input id="comparePlayer1" type="text" placeholder="Player 1" />
    <input id="comparePlayer2" type="text" placeholder="Player 2" />

    <select id="compareType">
      <option value="kd">K/D Ratio Trend (Last 30 Days)</option>
      <option value="win">Win Rate by Vehicle Type</option>
    </select>

    <button id="btnCompare">Compare</button>
  </div>

  <div id="compareResult" style="display:none;">
    <canvas id="compareChart" height="120"></canvas>
  </div>
</section>
<?php endif; ?>




<?php include "includes/footer.php"; ?>

<script>
  window.isLoginOrNot = <?= $isLoginOrNot ? 'true' : 'false' ?>;
</script>

<script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
<script src="assets/js/player.js?v=<?php echo time(); ?>" defer></script>