const brightnessSlider = document.getElementById('brightness-slider');
const flashSlider = document.getElementById('flash-slider');
const servoSlider = document.getElementById('servo-slider');
const brightnessValue = document.getElementById('brightness-value');
const flashValue = document.getElementById('flash-value');
const servoValue = document.getElementById('servo-value');
const statusDisplay = document.getElementById('status');

// Connect to WebSocket server
const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
const ws = new WebSocket(`${protocol}//${window.location.host}`);

ws.onopen = () => {
  console.log('Connected to server');
  statusDisplay.textContent = 'Connected';
  statusDisplay.style.color = 'green';
};

ws.onclose = () => {
  console.log('Disconnected from server');
  statusDisplay.textContent = 'Disconnected';
  statusDisplay.style.color = 'red';
};

ws.onerror = (error) => {
  console.error('WebSocket error:', error);
  statusDisplay.textContent = 'Error';
  statusDisplay.style.color = 'red';
};

ws.onmessage = (event) => {
  console.log('Message from server:', event.data);
  try {
    const data = JSON.parse(event.data);
    
    // Handle initial state from server
    if (data.type === 'initialState') {
      brightnessSlider.value = data.state.brightness;
      brightnessValue.textContent = data.state.brightness;
      
      flashSlider.value = data.state.pulseRate;
      flashValue.textContent = data.state.pulseRate;
      
      servoSlider.value = data.state.servoAngle;
      servoValue.textContent = data.state.servoAngle;
    }
    
    // Update brightness slider from other clients
    if (data.type === 'brightness' && data.value !== undefined) {
      brightnessSlider.value = data.value;
      brightnessValue.textContent = data.value;
    }
    
    // Update pulse slider from other clients
    if (data.type === 'pulse' && data.value !== undefined) {
      flashSlider.value = data.value;
      flashValue.textContent = data.value;
    }
    
    // Update servo slider from other clients
    if (data.type === 'servo' && data.value !== undefined) {
      servoSlider.value = data.value;
      servoValue.textContent = data.value;
    }
    
  } catch (error) {
    console.error('Error parsing message:', error);
  }
};

// Send brightness value to server
brightnessSlider.addEventListener('input', (e) => {
  const value = e.target.value;
  brightnessValue.textContent = value;
  if (ws.readyState === WebSocket.OPEN) {
    ws.send(JSON.stringify({
      type: 'brightness',
      value: parseInt(value)
    }));
  }
});

// Send pulse rate to server
flashSlider.addEventListener('input', (e) => {
  const value = e.target.value;
  flashValue.textContent = value;
  if (ws.readyState === WebSocket.OPEN) {
    ws.send(JSON.stringify({
      type: 'pulse',
      value: parseInt(value)
    }));
  }
});

// Send servo angle to server
servoSlider.addEventListener('change', (e) => { //note fire on change only, not live during the sliding
  const value = e.target.value;
  servoValue.textContent = value;
  if (ws.readyState === WebSocket.OPEN) {
    ws.send(JSON.stringify({
      type: 'servo',
      value: parseInt(value)
    }));
  }
});
