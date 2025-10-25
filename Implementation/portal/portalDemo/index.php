<?php 
    $page_title = "Home"; ?>
<?php 
    include "includes/header.php"; ?>

<!-- welcome!!!!! -->
<section class="hero">
    <h1>WELCOME TO WORLDREBALANCE</h1>
    <p>This is a platform for players and squadrons to view and analyze game data.</p>
    <div class="search-box">
    <input type="text" id="searchInput" placeholder="Enter player or vehicle name">
    <button id="searchBtn">Search</button>
</div>

<script>
document.getElementById('searchBtn').addEventListener('click', async () => {
    const query = document.getElementById('searchInput').value.trim();
    if (!query) {
        alert("Please enter a name!");
        return;
    }

    // 调用后端接口判断属于哪种类型
    const res = await fetch(`searchRedirect.php?q=${encodeURIComponent(query)}`);
    const data = await res.json();

    if (data.type === 'player') {
        window.location.href = `player.php?player=${encodeURIComponent(query)}`;
    } else if (data.type === 'vehicle') {
        window.location.href = `vehicle.php?name=${encodeURIComponent(query)}`;
    } else {
        alert("No matching player or vehicle found.");
    }
});
</script>
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

<?php include "includes/footer.php"; ?>
