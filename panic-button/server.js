const WebSocket = require('ws');
const PORT = 8080;

const wss = new WebSocket.Server({ port: PORT });
let currentStatus = { device_id: 'panic_001', status: 'idle' };

wss.on('connection', (ws) => {
  console.log('Client connected');
  // send current status
  ws.send(JSON.stringify(currentStatus));

  ws.on('message', (message) => {
    console.log('received: %s', message);
    try {
      const data = JSON.parse(message);
      // If device sends status update, broadcast to all clients
      if (data.device_id && data.status) {
        currentStatus = data;
        // broadcast
        wss.clients.forEach((c) => {
          if (c.readyState === WebSocket.OPEN) c.send(JSON.stringify(currentStatus));
        });
      }
      // If web client sends cancel command
      if (data.cmd && data.cmd === 'cancel') {
        // set neutral state
        currentStatus.status = 'neutral';
        // broadcast neutral status AND send a cancel command so devices react
        const payload = Object.assign({}, currentStatus, { cmd: 'cancel' });
        wss.clients.forEach((c) => {
          if (c.readyState === WebSocket.OPEN) c.send(JSON.stringify(payload));
        });
      }
    } catch (e) {
      console.error('Invalid JSON');
    }
  });

  ws.on('close', () => console.log('Client disconnected'));
});

console.log('WebSocket server listening on port', PORT);