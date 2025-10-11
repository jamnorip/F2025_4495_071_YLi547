<?php $page_title = "Vehicles"; ?>
<?php include __DIR__ . "/includes/header.php"; ?>


<section class="hero">
  <h1>Vehicle stats for 1 month</h1>
  <div class="filter-bar">
    <label>Stats for last</label>
    <select>
      <option>1 month</option>
      <option>3 months</option>
      <option>6 months</option>
    </select>
  </div>
</section>

<!-- selection -->
<section class="filters">
  <div class="search-bar">
    <input type="text" placeholder="Search vehicles...">
  </div>
  <div class="country-buttons">
    <button>All</button>
    <button>🇬🇧 Britain</button>
    <button>🇨🇳 China</button>
    <button>🇫🇷 France</button>
    <button>🇩🇪 Germany</button>
    <button>🇺🇸 USA</button>
    <button>🇷🇺 USSR</button>
  </div>
  <div class="role-buttons">
    <button>All roles</button>
    <button>Main Battle Tanks</button>
    <button>Light Tanks</button>
  </div>
</section>

<!-- 表格数据 -->
<section class="vehicle-table">
  <table>
    <thead>
      <tr>
        <th>Rank</th>
        <th>Name</th>
        <th>Role</th>
        <th>Matches Played</th>
        <th>Wins</th>
        <th>K/D Ratio</th>
      </tr>
    </thead>
    <tbody>
      <tr>
        <td>1</td>
        <td>T-90A</td>
        <td>Main Battle Tank</td>
        <td>150</td>
        <td>92</td>
        <td>1.5</td>
      </tr>
      <tr>
        <td>2</td>
        <td>M1A2 Abrams</td>
        <td>Main Battle Tank</td>
        <td>210</td>
        <td>130</td>
        <td>1.7</td>
      </tr>
      <tr>
        <td>3</td>
        <td>Leopard 2A6</td>
        <td>Main Battle Tank</td>
        <td>198</td>
        <td>125</td>
        <td>1.6</td>
      </tr>
    </tbody>
  </table>
</section>

<?php include __DIR__ . "/includes/footer.php"; ?>
