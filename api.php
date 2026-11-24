<?php
/**
 * Dollynho Toy - Cloud API Bridge
 * Comunicação Remota entre o Dashboard na Hostinger e o ESP32-S3 via Wi-Fi
 */

header("Access-Control-Allow-Origin: *");
header("Access-Control-Allow-Methods: GET, POST, OPTIONS");
header("Access-Control-Allow-Headers: Content-Type");
header("Content-Type: application/json; charset=UTF-8");

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

$queueFile  = __DIR__ . '/cmd_queue.json';
$statusFile = __DIR__ . '/esp_status.json';

// Lê o corpo da requisição JSON caso enviado via POST
$inputRaw = file_get_contents('php://input');
$inputData = json_decode($inputRaw, true) ?: [];

$action = $_GET['action'] ?? $inputData['action'] ?? '';
$cmd    = $_GET['cmd'] ?? $inputData['cmd'] ?? '';
$id     = $_GET['id'] ?? $inputData['id'] ?? '';

switch ($action) {
    // -------------------------------------------------------------
    // 1. Enviar Comando do Dashboard para o Robô (Nuvem)
    // -------------------------------------------------------------
    case 'send_cmd':
        if (empty($cmd)) {
            echo json_encode(['success' => false, 'error' => 'Comando vazio']);
            exit;
        }

        // Ignora 'blink' caso venha de abas de navegador antigas em cache
        if (strtolower(trim($cmd)) === 'blink') {
            echo json_encode(['success' => true, 'ignored' => true, 'message' => 'Blink automatico ignorado']);
            exit;
        }

        $queue = [];
        if (file_exists($queueFile)) {
            $data = json_decode(file_get_contents($queueFile), true);
            if (is_array($data)) {
                $queue = isset($data['cmd']) ? [$data] : $data;
            }
        }

        // Limpa itens antigos (> 60 segundos)
        $now = time();
        $queue = array_values(array_filter($queue, function($item) use ($now) {
            return ($now - ($item['timestamp'] ?? 0)) < 60;
        }));

        $cmdItem = [
            'id'        => microtime(true),
            'cmd'       => trim($cmd),
            'timestamp' => $now,
            'executed'  => false
        ];
        $queue[] = $cmdItem;

        // Mantém no máximo os 10 últimos comandos
        if (count($queue) > 10) {
            $queue = array_slice($queue, -10);
        }

        file_put_contents($queueFile, json_encode($queue));

        echo json_encode([
            'success' => true,
            'message' => 'Comando enfileirado na nuvem com sucesso!',
            'item'    => $cmdItem
        ]);
        break;

    // -------------------------------------------------------------
    // 2. Polling do ESP32: Verifica se há comando pendente + Heartbeat
    // -------------------------------------------------------------
    case 'poll':
        $ip   = $_GET['ip'] ?? $inputData['ip'] ?? '0.0.0.0';
        $rssi = (int)($_GET['rssi'] ?? $inputData['rssi'] ?? 0);
        $mood = $_GET['mood'] ?? $inputData['mood'] ?? 'DEFAULT';

        // Atualiza status do ESP32 (Heartbeat)
        $statusData = [
            'online'        => true,
            'last_seen'     => time(),
            'ip'            => $ip,
            'rssi'          => $rssi,
            'mood'          => $mood,
            'user_agent'    => $_SERVER['HTTP_USER_AGENT'] ?? 'ESP32'
        ];
        file_put_contents($statusFile, json_encode($statusData));

        // Checa se há comando na fila aguardando execução
        $pendingCmd = null;
        if (file_exists($queueFile)) {
            $data = json_decode(file_get_contents($queueFile), true);
            $queue = [];
            if (is_array($data)) {
                $queue = isset($data['cmd']) ? [$data] : $data;
            }

            $now = time();
            $updated = false;
            foreach ($queue as &$item) {
                if (empty($item['executed']) && ($now - ($item['timestamp'] ?? 0) < 45)) {
                    $pendingCmd = [
                        'cmd' => $item['cmd'],
                        'id'  => $item['id']
                    ];
                    if (!empty($_GET['auto_ack']) || !empty($inputData['auto_ack'])) {
                        $item['executed'] = true;
                        $item['ack_time'] = $now;
                        $updated = true;
                    }
                    break; // Pega o primeiro comando pendente na fila FIFO
                }
            }
            if ($updated) {
                file_put_contents($queueFile, json_encode($queue));
            }
        }

        echo json_encode([
            'status' => 'ok',
            'cmd'    => $pendingCmd ? $pendingCmd['cmd'] : null,
            'id'     => $pendingCmd ? $pendingCmd['id'] : null
        ]);
        break;

    // -------------------------------------------------------------
    // 3. Confirmação de Execução (ACK) pelo ESP32
    // -------------------------------------------------------------
    case 'ack':
        if (file_exists($queueFile)) {
            $data = json_decode(file_get_contents($queueFile), true);
            $queue = is_array($data) ? (isset($data['cmd']) ? [$data] : $data) : [];
            $now = time();
            foreach ($queue as &$item) {
                if (!empty($id) && isset($item['id']) && $item['id'] == $id) {
                    $item['executed'] = true;
                    $item['ack_time'] = $now;
                } else if (empty($id) && empty($item['executed'])) {
                    $item['executed'] = true;
                    $item['ack_time'] = $now;
                }
            }
            file_put_contents($queueFile, json_encode($queue));
        }
        echo json_encode(['success' => true, 'message' => 'Comando confirmado']);
        break;

    // -------------------------------------------------------------
    // 4. Status em Tempo Real para o Dashboard Web
    // -------------------------------------------------------------
    case 'status':
    default:
        $espStatus = [
            'online'            => false,
            'last_seen_sec_ago' => 9999,
            'ip'                => null,
            'rssi'              => 0,
            'mood'              => 'DEFAULT'
        ];

        if (file_exists($statusFile)) {
            $data = json_decode(file_get_contents($statusFile), true);
            if (!empty($data)) {
                $secAgo = time() - ($data['last_seen'] ?? 0);
                $espStatus = [
                    'online'            => ($secAgo <= 8), // Considera online se respondeu nos últimos 8s
                    'last_seen_sec_ago' => $secAgo,
                    'ip'                => $data['ip'] ?? null,
                    'rssi'              => $data['rssi'] ?? 0,
                    'mood'              => $data['mood'] ?? 'DEFAULT'
                ];
            }
        }

        // Lê último comando da fila
        if (file_exists($queueFile)) {
            $data = json_decode(file_get_contents($queueFile), true);
            if (is_array($data)) {
                $lastItem = isset($data['cmd']) ? $data : end($data);
                if (!empty($lastItem)) {
                    $espStatus['last_cmd'] = $lastItem['cmd'] ?? null;
                    $espStatus['last_cmd_executed'] = !empty($lastItem['executed']);
                }
            }
        }

        echo json_encode($espStatus);
        break;
}
