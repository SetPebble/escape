//  file: pebble-js-app.js
//  auth: Matthew Clark, SetPebble

var messages, communicating, debug = false;

//  message functions

function message_process() {
  //  check message queue
  if (messages.length > 0) {
    //  check communicating flag
    if (!communicating) {
      //  set communicating flag
      communicating = true;
      //  get message to send
      var message = messages[0];
      //  remove from queue
      messages.shift();
      //  send message
      Pebble.sendAppMessage(
        message,
        function(e) {   //  success
          if (debug)
            console.log(JSON.stringify(message));
          //  clear communicating flag
          communicating = false;
          //  check for another message
          message_process();
        },
        function(e) {   //  failure
          //  clear communicating flag
          communicating = false;
          //  insert message to front of message queue
          messages.splice(0, null, message);
          //  send again
          message_process();
        }
      );
    }
  }
}

function message_queue(message) {
  //  add copy of message to queue
  if (Object.prototype.toString.call(messages) !== '[object Array]')
    messages = [];
  messages.push(JSON.parse(JSON.stringify(message)));
  //  check queue
  message_process();
}

//  event handlers

Pebble.addEventListener('ready', function(arg) {
  messages = [];
  communicating = false;
});
Pebble.addEventListener('appmessage', function(arg) {
  //  get local settings
  var settings = localStorage.getItem('KEY');
  if (typeof(settings) == 'string')
    message_queue(JSON.parse(settings));
  //  get remote settings
  var request = new XMLHttpRequest();
  request.open('GET', 'http://x.SetPebble.com/api/KEY/' + Pebble.getAccountToken(), true);
  request.onload = function() {
    if ((request.readyState == 4) && (request.status == 200)) {
      if ((typeof(request.responseText) == 'string') && (request.responseText.length > 0)) {
        //  send to Pebble
        message_queue(JSON.parse(request.responseText));
        //  store locally
        localStorage.setItem('KEY', request.responseText);
      }
    }
  };
  request.send(null);
});
Pebble.addEventListener('showConfiguration', function(e) {
  //  open configuration panel
  Pebble.openURL('http://x.SetPebble.com/KEY/' + Pebble.getAccountToken());
});
Pebble.addEventListener('webviewclosed', function(arg) {
  if ((typeof(arg.response) == 'string') && (arg.response.length > 0)) {
    //  send to Pebble
    message_queue(JSON.parse(arg.response));
    //  store locally
    localStorage.setItem('KEY', arg.response);
  }
});