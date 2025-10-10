<?php 
    $page_title = "Players"; ?>
<?php 
    include __DIR__ . "/includes/header.php"; ?>


<section class="hero">
  <h1>Average Efficiency of Players</h1>
  <p>Overview of player performance in Arcade, Realistic, and Simulator battles.</p>
</section>

<!-- !!!!test data -->
<section class="charts" style="display:flex; justify-content:center; flex-wrap:wrap; gap:30px; padding:40px;">
  <div class="feature-box" style="min-width:300px;">
    <i class="fa-solid fa-gamepad fa-2x"></i>
    <h3>Arcade Battles</h3>
    <img src="assets/img/mock_arcade_chart.png" alt="Arcade Chart" style="width:100%; border-radius:6px;">
  </div>

  <div class="feature-box" style="min-width:300px;">
    <i class="fa-solid fa-fighter-jet fa-2x"></i>
    <h3>Realistic Battles</h3>
    <img src="assets/img/mock_realistic_chart.png" alt="Realistic Chart" style="width:100%; border-radius:6px;">
  </div>

  <div class="feature-box" style="min-width:300px;">
    <i class="fa-solid fa-vr-cardboard fa-2x"></i>
    <h3>Simulator Battles</h3>
    <img src="assets/img/mock_simulator_chart.png" alt="Simulator Chart" style="width:100%; border-radius:6px;">
  </div>
</section>

<!-- avg area -->
<section class="averages" style="text-align:center; padding:20px;">
  <h2 style="color:#e6e600;">Battle Averages</h2>
  <div style="display:flex; justify-content:center; flex-wrap:wrap; gap:30px; margin-top:20px;">
    
    <div class="feature-box" style="width:300px;">
      <h3>Arcade Battles</h3>
      <p>Win Rate: <b>64.8%</b></p>
      <p>K/D Ratio: <b>2.2</b></p>
      <p>Frags per Battle: <b>4.3</b></p>
      <p>Lifespan: <b>6 min</b></p>
      <p>Average Battles: <b>2250</b></p>
    </div>

    <div class="feature-box" style="width:300px;">
      <h3>Realistic Battles</h3>
      <p>Win Rate: <b>62.7%</b></p>
      <p>K/D Ratio: <b>1.8</b></p>
      <p>Frags per Battle: <b>2.3</b></p>
      <p>Lifespan: <b>8 min</b></p>
      <p>Average Battles: <b>5613</b></p>
    </div>

    <div class="feature-box" style="width:300px;">
      <h3>Simulator Battles</h3>
      <p>Win Rate: <b>62.4%</b></p>
      <p>K/D Ratio: <b>1.6</b></p>
      <p>Frags per Battle: <b>2.0</b></p>
      <p>Lifespan: <b>8 min</b></p>
      <p>Average Battles: <b>122</b></p>
    </div>
  </div>
</section>

<!-- level display -->
<section style="text-align:center; padding:40px;">
  <h2 style="color:#e6e600;">Efficiency Required for Each Rank</h2>
  <div style="display:flex; justify-content:center; flex-wrap:wrap; gap:30px; margin-top:20px;">
    <div class="feature-box" style="width:300px;">
      <h3>Arcade Battles</h3>
      <img src="assets/img/mock_arcade_ranks.png" alt="Arcade Ranks" style="width:100%; border-radius:6px;">
    </div>
    <div class="feature-box" style="width:300px;">
      <h3>Realistic Battles</h3>
      <img src="assets/img/mock_realistic_ranks.png" alt="Realistic Ranks" style="width:100%; border-radius:6px;">
    </div>
    <div class="feature-box" style="width:300px;">
      <h3>Simulator Battles</h3>
      <img src="assets/img/mock_simulator_ranks.png" alt="Simulator Ranks" style="width:100%; border-radius:6px;">
    </div>
  </div>
</section>

<?php include __DIR__ . "/includes/footer.php"; ?>

