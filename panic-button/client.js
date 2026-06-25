(function(){
  const host = location.hostname || 'localhost';
  const port = 8080;
  let ws;
  const statEl = document.getElementById('stat');
  const didEl = document.getElementById('did');
  const statusWrapper = document.getElementById('status');
  const cancelBtn = document.getElementById('cancel');

  function setStatus(s, device){
    statEl.textContent = s;
    if (device) didEl.textContent = device;
    statusWrapper.setAttribute('data-state', s);
    // indicator
    if (!statusWrapper.querySelector('.indicator')){
      const dot = document.createElement('span');
      dot.className = 'indicator';
      statusWrapper.appendChild(dot);
    }
    const dot = statusWrapper.querySelector('.indicator');
    dot.className = 'indicator ' + (s === 'panic' ? 'panic' : (s === 'neutral' ? 'neutral' : 'idle'));
  }

  function connect(){
    ws = new WebSocket('ws://' + host + ':' + port);
    ws.onopen = () => console.log('WS open');
    ws.onmessage = (ev) => {
      try{
        const data = JSON.parse(ev.data);
        setStatus(data.status || 'unknown', data.device_id);
      }catch(e){ console.error(e); }
    };
    ws.onclose = () => { console.log('WS closed, reconnect in 2s'); setTimeout(connect, 2000); };
    ws.onerror = (e) => { console.error('WS error', e); ws.close(); };
  }

  cancelBtn.addEventListener('click', function(){
    if (ws && ws.readyState === WebSocket.OPEN){
      ws.send(JSON.stringify({ cmd: 'cancel' }));
    }
  });

  // start
  connect();
  // set an initial visual state
  setStatus('idle', didEl.textContent);
})();
