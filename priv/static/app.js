var cfSocket = new WebSocket("ws://localhost:4000/ws");
cfSocket.onmessage = function (event) {
  cfSocket.send("ping");
  var stuff = JSON.parse(event.data);
  var object = stuff.data;
  for (var property in object) {
    if(property == "params") return;
    if (object.hasOwnProperty(property)) {
        var elem = $('[name="'+ property + '_value' + '"]')[0];
        if(elem) {
          elem.innerHTML = object[property];
        } else {
          console.warn("unknown property: " + property);
        }
      }
  }
}
