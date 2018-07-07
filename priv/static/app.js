var cfSocket = new WebSocket("ws://localhost:4000/ws");
cfSocket.onmessage = function (event) {
  var data = JSON.parse(event.data);
  console.dir(data);
}
