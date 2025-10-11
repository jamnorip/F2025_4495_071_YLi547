<?php 
    $page_title = "Home"; ?>
<?php 
    include __DIR__ . "/includes/header.php"; ?>

<!-- welcome!!!!! -->
<section class="hero">
    <h1>WELCOME TO WORLDREBALANCE</h1>
    <p>This is a platform for players and squadrons to view and analyze game data.</p>
    <div class="search-box">
        <input type="text" placeholder="Enter nickname or squad name">
        <button>Search</button>
    </div>
</section>

<!-- test modals -->
<section class="features">
    <div class="feature-box">
        <i class="fa-solid fa-gear fa-3x"></i>
        <h3>Vehicle Statistics</h3>
        <button>View</button>
    </div>
    <div class="feature-box">
        <i class="fa-solid fa-crown fa-3x"></i>
        <h3>Squad Ranks</h3>
        <button>View</button>
    </div>
    <div class="feature-box">
        <i class="fa-solid fa-chart-line fa-3x"></i>
        <h3>Player Statistics</h3>
        <button>View</button>
    </div>
    <div class="feature-box">
        <i class="fa-solid fa-video fa-3x"></i>
        <h3>WT Videos</h3>
        <button>View</button>
    </div>
</section>

<?php include __DIR__ . "/includes/footer.php"; ?>
