<?php require_once __DIR__ . "/config.php"; ?>
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <title><?= isset($page_title) ? $page_title . " | " . $site_title : $site_title ?></title>

  
  <link rel="stylesheet" href="<?= url('assets/css/style.css') ?>" />
  <link href="https://fonts.googleapis.com/css2?family=Roboto&display=swap" rel="stylesheet" />
  <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.4.0/css/all.min.css" />
</head>
<body>
<header>
  <!-- navegation area -->
  <nav>
    <div class="logo">WORLD<span>REBALANCE</span></div>
    <ul class="nav-links">
      <li><a <?= nav_active('index.php') ?> href="<?= url('index.php') ?>">Home</a></li>
      <li><a <?= nav_active('players.php') ?> href="<?= url('players.php') ?>">Players</a></li>
      <li><a <?= nav_active('vehicles.php') ?> href="<?= url('vehicles.php') ?>">Vehicles</a></li>
      <li><a <?= nav_active('squad_ranks.php') ?> href="<?= url('squad_ranks.php') ?>">Squad Ranks</a></li>
      <li><a <?= nav_active('about.php') ?> href="<?= url('about.php') ?>">About</a></li>
    </ul>
    <div class="auth-buttons">
      <button>Login</button>
      <button>Sign Up</button>
    </div>
  </nav>
</header>
<main>
