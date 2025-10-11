<?php

$site_title = "World Rebalance Portal";

$base_url = rtrim(dirname($_SERVER['SCRIPT_NAME']), '/\\');
if ($base_url === '/' || $base_url === '\\' || $base_url === '.' ) {
    $base_url = '';
}

function url($path) {
    global $base_url;
    return ($base_url ? $base_url : '') . '/' . ltrim($path, '/');
}


function nav_active($file) {
    $current = basename($_SERVER['SCRIPT_NAME']);
    return $current === $file ? 'class="active"' : '';
}
