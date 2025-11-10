<?php 
$page_title = "Home"; 
include "includes/header.php"; 
?>

<!-- welcome!!!!! -->
<section class="hero">
    <h1>WELCOME TO WORLDREBALANCE</h1>
    <p>This is a platform for players and squadrons to view and analyze game data.</p>
    <div class="search-box">
        <input type="text" id="searchInput" placeholder="Enter player or vehicle name">
        <button id="searchBtn">Search</button>
    </div>
</section>

<!-- search logic -->
<script>
document.getElementById('searchBtn').addEventListener('click', async () => {
    const query = document.getElementById('searchInput').value.trim();
    if (!query) {
        alert("Please enter a name!");
        return;
    }

    try {
        const res = await fetch(`searchRedirect.php?q=${encodeURIComponent(query)}`);
        const data = await res.json();

        if (data.type === 'player') {
            window.location.href = `players.php?player=${encodeURIComponent(data.name)}`;
        } else if (data.type === 'vehicle') {
            window.location.href = `vehicles.php?name=${encodeURIComponent(data.name)}`;
        } else {
            alert("No matching player or vehicle found.");
        }
    } catch (err) {
        alert("Search failed: " + err);
    }
});
</script>


<!-- test modals -->
<section class="features">
    <div class="feature-box">
        <i class="fa-solid fa-gear fa-3x"></i>
        <h3>Vehicle Statistics</h3>
        <button onclick="location.href='vehicles.php'">View</button>
    </div>
    <div class="feature-box">
        <i class="fa-solid fa-crown fa-3x"></i>
        <h3>Squad Ranks</h3>
        <button onclick="location.href='squad_ranks.php'">View</button>
    </div>
    <div class="feature-box">
        <i class="fa-solid fa-chart-line fa-3x"></i>
        <h3>Player Statistics</h3>
        <button onclick="location.href='players.php'">View</button>
    </div>
    <div class="feature-box">
        <i class="fa-solid fa-video fa-3x"></i>
        <h3>WT Videos</h3>
        <button onclick="location.href='videos.php'">View</button>
    </div>
</section>

<?php include "includes/footer.php"; ?>
