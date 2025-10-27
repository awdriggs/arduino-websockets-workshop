// one server to rule them all!
import express from 'express';
import { createServer } from 'http';
import { WebSocketServer } from 'ws';
import path from 'path';
import { fileURLToPath } from 'url';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

const app = express();
const server = createServer(app);
const wss = new WebSocketServer({ server });

app.use(express.static('public'));

const clients = new Set(); //a js storage object, similiar to array, but will prevent duplicate data

const serverState = {
  // Example 1 state
  ledOn: false,
  
  // Example 2 state
  brightness: 128,
  pulseRate: 50,
  servoAngle: 90
};

//helpter function for brodcasting data to clients
function broadcast(data) {
  const message = JSON.stringify(data);
  clients.forEach(client => {
    if (client.readyState === 1) {
      client.send(message);
    }
  });
}

wss.on('connection', (ws, req) => { //ws is the connected client
  console.log('New client connected');
  clients.add(ws); //add the connected client to the clients set
  
  // Send current state to newly connected client
  ws.send(JSON.stringify({
    type: 'initialState',
    state: serverState
  }));

  //add these event listeners to the client
  ws.on('message', (incomingData) => {
    try {
      const data = JSON.parse(incomingData); //incomingData string as json
      console.log('Received:', data); //peek at the incoming data
      
      // Example 1: Button toggle
      if (data.type === 'buttonPress') {
        serverState.ledOn = !serverState.ledOn; //toggle the led state
        console.log('Button toggled to:', serverState.buttonPressed);
        broadcast({ type: 'ledState', value: serverState.ledOn });
      }
      
      // Example 2: Slider controls
      if (data.type === 'brightness') {
        serverState.brightness = data.value;
        broadcast({ type: 'brightness', value: data.value });
      }
      
      if (data.type === 'pulse') {
        serverState.pulseRate = data.value;
        broadcast({ type: 'pulse', value: data.value });
      }
      
      if (data.type === 'servo') {
        serverState.servoAngle = data.value;
        broadcast({ type: 'servo', value: data.value });
      }
      
    } catch (error) {
      console.error('Error parsing message:', error);
    }
  });

  ws.on('close', () => {
    console.log('Client disconnected');
    clients.delete(ws);
  });

  ws.on('error', (error) => {
    console.error('WebSocket error:', error);
  });
});

const PORT = process.env.PORT || 3000;
server.listen(PORT, () => {
  console.log(`Server running on http://localhost:${PORT}`);
});
