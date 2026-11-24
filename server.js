const http = require('http');
const fs = require('fs');
const path = require('path');
const { SerialPort } = require('serialport');
const { ReadlineParser } = require('@serialport/parser-readline');

const PORT = 8080;
const PUBLIC_DIR = __dirname;
const SERIAL_BAUD = 115200;

let serialPort = null;
let parser = null;
let isConnected = false;
let currentPortName = null;
const logHistory = [];
const MAX_LOGS = 200;
const sseClients = new Set();

function addLog(text, source = 'esp32') {
    const time = new Date().toLocaleTimeString('pt-BR');
    const logEntry = { time, text: text.trim(), source };
    logHistory.push(logEntry);
    if (logHistory.length > MAX_LOGS) {
        logHistory.shift();
    }
    // Broadcast em tempo real para todos os navegadores conectados
    const payload = `data: ${JSON.stringify(logEntry)}\n\n`;
    for (const client of sseClients) {
        try {
            client.write(payload);
        } catch (e) {
            sseClients.delete(client);
        }
    }
}

// Inicializa conexão serial automática com o ESP32-S3
function initSerial() {
    SerialPort.list().then(ports => {
        // Procura por COM8 ou qualquer porta USB serial disponível
        const targetPort = ports.find(p => p.path === 'COM8') || 
                           ports.find(p => (p.pnpId && p.pnpId.includes('303A:1001')) ||
                                           (p.manufacturer && p.manufacturer.toLowerCase().includes('espressif')) ||
                                           p.path.startsWith('COM'));

        if (!targetPort) {
            console.log('[SERIAL] Nenhuma porta COM disponível. Tentando novamente em 3s...');
            setTimeout(initSerial, 3000);
            return;
        }

        const portPath = targetPort.path;
        currentPortName = portPath;
        console.log(`[SERIAL] Conectando ao ESP32 na porta ${portPath} (${SERIAL_BAUD} baud)...`);

        try {
            serialPort = new SerialPort({
                path: portPath,
                baudRate: SERIAL_BAUD,
                autoOpen: false,
                rtscts: false
            });

            parser = serialPort.pipe(new ReadlineParser({ delimiter: '\n' }));

            serialPort.open((err) => {
                if (err) {
                    console.error(`[SERIAL] Erro ao abrir ${portPath}:`, err.message);
                    isConnected = false;
                    addLog(`[SISTEMA] Falha ao abrir ${portPath}: ${err.message}`, 'system');
                    setTimeout(initSerial, 4000);
                    return;
                }
                isConnected = true;
                console.log(`[SERIAL] Conectado com sucesso ao ESP32 em ${portPath}!`);
                addLog(`[SISTEMA] Conectado com sucesso ao ESP32 (${portPath} @ ${SERIAL_BAUD}bps)`, 'system');

                // Envia a hora atual para sincronizar o visor OLED do robô
                setTimeout(() => {
                    const now = new Date();
                    const hh = String(now.getHours()).padStart(2, '0');
                    const mm = String(now.getMinutes()).padStart(2, '0');
                    sendToEsp32(`time:${hh}:${mm}`);
                }, 1500);
            });

            parser.on('data', (line) => {
                const clean = line.replace(/[\r\n]+/g, '');
                if (clean) {
                    console.log(`[ESP32] ${clean}`);
                    addLog(clean, 'esp32');
                }
            });

            serialPort.on('close', () => {
                console.log('[SERIAL] Porta serial fechada.');
                isConnected = false;
                addLog('[SISTEMA] Conexão serial com ESP32 perdida. Reconectando...', 'system');
                setTimeout(initSerial, 3000);
            });

            serialPort.on('error', (err) => {
                console.error('[SERIAL] Erro:', err.message);
                isConnected = false;
            });

        } catch (e) {
            console.error('[SERIAL] Exceção:', e.message);
            setTimeout(initSerial, 3000);
        }
    }).catch(err => {
        console.error('[SERIAL] Erro ao listar portas:', err);
        setTimeout(initSerial, 3000);
    });
}

function sendToEsp32(cmd) {
    if (!serialPort || !serialPort.isOpen) {
        return { success: false, error: 'ESP32 não conectado à porta serial' };
    }
    serialPort.write(cmd + '\n');
    addLog(`[ENVIADO] ➔ ${cmd}`, 'sent');
    return { success: true, cmd };
}

// Servidor HTTP
const server = http.createServer((req, res) => {
    // Rota SSE: Stream de Logs em Tempo Real
    if (req.url === '/api/events') {
        res.writeHead(200, {
            'Content-Type': 'text/event-stream',
            'Cache-Control': 'no-cache',
            'Connection': 'keep-alive',
            'Access-Control-Allow-Origin': '*'
        });
        res.write('retry: 2000\n\n');
        sseClients.add(res);

        // Envia os logs recentes acumulados
        for (const log of logHistory.slice(-40)) {
            res.write(`data: ${JSON.stringify(log)}\n\n`);
        }

        req.on('close', () => {
            sseClients.delete(res);
        });
        return;
    }

    // Rota GET /api/status
    if (req.url === '/api/status') {
        res.writeHead(200, {
            'Content-Type': 'application/json; charset=utf-8',
            'Access-Control-Allow-Origin': '*'
        });
        res.end(JSON.stringify({
            connected: isConnected,
            port: currentPortName,
            baudRate: SERIAL_BAUD,
            logsCount: logHistory.length
        }));
        return;
    }

    // Rota GET /api/logs
    if (req.url === '/api/logs') {
        res.writeHead(200, {
            'Content-Type': 'application/json; charset=utf-8',
            'Access-Control-Allow-Origin': '*'
        });
        res.end(JSON.stringify(logHistory));
        return;
    }

    // Rota POST /api/send
    if (req.url === '/api/send' && req.method === 'POST') {
        let body = '';
        req.on('data', chunk => { body += chunk; });
        req.on('end', () => {
            try {
                const data = JSON.parse(body || '{}');
                const cmd = (data.cmd || '').trim();
                if (!cmd) {
                    res.writeHead(400, { 'Content-Type': 'application/json' });
                    res.end(JSON.stringify({ success: false, error: 'Comando vazio' }));
                    return;
                }
                const result = sendToEsp32(cmd);
                res.writeHead(result.success ? 200 : 503, {
                    'Content-Type': 'application/json; charset=utf-8',
                    'Access-Control-Allow-Origin': '*'
                });
                res.end(JSON.stringify(result));
            } catch (e) {
                res.writeHead(400, { 'Content-Type': 'application/json' });
                res.end(JSON.stringify({ success: false, error: e.message }));
            }
        });
        return;
    }

    // Arquivos Estáticos (index.html, etc.)
    let cleanUrl = req.url.split('?')[0];
    let filePath = path.join(PUBLIC_DIR, cleanUrl === '/' ? 'index.html' : cleanUrl);
    fs.readFile(filePath, (err, data) => {
        if (err) {
            res.writeHead(404, { 'Content-Type': 'text/plain; charset=utf-8' });
            res.end('404 Not Found');
            return;
        }
        let ext = path.extname(filePath);
        let contentType = 'text/html';
        if (ext === '.js') contentType = 'text/javascript';
        if (ext === '.css') contentType = 'text/css';
        if (ext === '.json') contentType = 'application/json';
        if (ext === '.stl') contentType = 'application/octet-stream';

        res.writeHead(200, { 'Content-Type': `${contentType}; charset=utf-8` });
        res.end(data);
    });
});

server.listen(PORT, '0.0.0.0', () => {
    console.log(`[SERVIDOR] Dashboard rodando em http://localhost:${PORT}`);
    initSerial();
});
