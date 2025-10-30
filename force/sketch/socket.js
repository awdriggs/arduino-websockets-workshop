let forceValue = 0;
let socketConnected = false;

// Connect to WebSocket server
const protocol = 'wss:';
const url = 'arduino-websockets-workshop.onrender.com'
const ws = new WebSocket(`${protocol}//${url}`);
ws.onopen = () => {
  console.log('Connected to server');
  socketConnected = true;
};

ws.onclose = () => {
  console.log('Disconnected from server');
  socketConnected = false;
};

ws.onerror = (error) => {
  console.error('WebSocket error:', error);
};

ws.onmessage = (event) => {
  // console.log('Message from server:', event.data);
  try {
    const data = JSON.parse(event.data);
    
    // Handle initial state from server
    if (data.type === 'initialState') {
      forceValue = data.state.brightness
    }
    
    // Update brightness slider from other clients
    if (data.type === 'brightness' && data.value !== undefined) {
      forceValue = data.value;
    }
  } catch (error) {
    console.error('Error parsing message:', error);
  }
};

