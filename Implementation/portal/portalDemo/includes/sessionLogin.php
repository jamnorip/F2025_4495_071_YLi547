<?php
// 注意：文件头部不要有 BOM 或空格！
require_once __DIR__ . '/session.php';

// 只返回 JSON，避免任何 HTML
header('Content-Type: application/json; charset=utf-8');

// 如果有报错就返回 JSON（可在开发阶段打开，发布时可关闭）
// ini_set('display_errors', 0);
// error_reporting(0);

// 读取 JSON 请求体
$raw = file_get_contents('php://input');
$data = json_decode($raw, true);

// 验证 JSON
if (json_last_error() !== JSON_ERROR_NONE) {
    http_response_code(400);
    echo json_encode(['ok' => false, 'error' => 'invalid_json', 'raw' => $raw], JSON_UNESCAPED_UNICODE);
    exit;
}

$username = trim($data['username'] ?? '');
if ($username === '') {
    http_response_code(400);
    echo json_encode(['ok' => false, 'error' => 'missing_username'], JSON_UNESCAPED_UNICODE);
    exit;
}

// 写入会话
$_SESSION['username']    = $username;
$_SESSION['displayName'] = $data['displayName'] ?? '';
$_SESSION['uid']         = $data['uid'] ?? '';

// 返回 JSON
echo json_encode([
    'ok' => true,
    'username' => $_SESSION['username']
], JSON_UNESCAPED_UNICODE);
exit;
