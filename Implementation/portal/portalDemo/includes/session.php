<?php
//全局 Session 启动文件（所有页面共用）
if (session_status() === PHP_SESSION_NONE) {
    session_start();
}
?>