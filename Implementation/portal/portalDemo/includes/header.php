<?php 
require_once __DIR__ . "/config.php"; 
require_once 'includes/session.php';
//全局防缓存设置，避免浏览器回退显示旧页面
header('Cache-Control: no-store, no-cache, must-revalidate, max-age=0');
header('Pragma: no-cache');
header('Expires: 0');


?>
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
        <span id="user-info" style="margin-right:10px; color:yellow; font-weight:bold;"></span>
        <button id="btn-login" onclick="location.href='login.html'">Login</button>
        <button id="btn-signup" onclick="location.href='signup.html'">Sign Up</button>
        <button id="btn-logout" style="display:none;">Logout</button>
      </div>
    </nav>
  </header>

  <main>

  <!-- ✅ Firebase 登录状态控制脚本 -->
  <script type="module">
    import { initializeApp } from "https://www.gstatic.com/firebasejs/11.0.0/firebase-app.js";
    import { getAuth, onAuthStateChanged, signOut } from "https://www.gstatic.com/firebasejs/11.0.0/firebase-auth.js";

    const firebaseConfig = {
      apiKey: "AIzaSyAxhd0AfXSzbQac2wfVKUgaSHVvzdd4cls",
      authDomain: "worldbalance-fd82b.firebaseapp.com",
      projectId: "worldbalance-fd82b",
      storageBucket: "worldbalance-fd82b.appspot.com",
      messagingSenderId: "644297876337",
      appId: "1:644297876337:web:64cfd5587c693cca0f8cd0"
    };

    // ✅ 初始化 Firebase
    const app = initializeApp(firebaseConfig);
    const auth = getAuth(app);

    const loginBtn = document.getElementById('btn-login');
    const signupBtn = document.getElementById('btn-signup');
    const logoutBtn = document.getElementById('btn-logout');
    const userInfo = document.getElementById('user-info');

    // ✅ 登录状态变化监听
    onAuthStateChanged(auth, async (user) => {
      console.log("Auth state:", user ? user.email : "not logged in");

      if (user) {
        // 登录状态：显示邮箱和 Logout 按钮
        userInfo.textContent = `Welcome, ${user.email}`;
        loginBtn.style.display = 'none';
        signupBtn.style.display = 'none';
        logoutBtn.style.display = 'inline-block';

        //这里发送请求到sessionLogin.php，写入php:input
        try {
          const res = await fetch('includes/sessionLogin.php', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            credentials: 'same-origin', // 保证 PHP session 正常保存
            body: JSON.stringify({
              username: user.email,
              displayName: user.displayName || '',
              uid: user.uid || ''
            })
          });
          const data = await res.json();
          console.log("PHP session response:", data);
          console.log(data.username)
        } catch (err) {
          console.error("Session sync failed:", err);
        }

      } else {
        // 未登录状态：只显示登录/注册
        userInfo.textContent = '';
        loginBtn.style.display = 'inline-block';
        signupBtn.style.display = 'inline-block';
        logoutBtn.style.display = 'none';
      }
    });

    //logout
    logoutBtn.addEventListener('click', async () => {
      await signOut(auth);
      // 清除 PHP session
      await fetch('includes/logout_session.php', { method: 'POST', credentials: 'same-origin' });

      alert('You have logged out.');
      location.href = 'index.php';
    });
  </script>

</body>
</html>
