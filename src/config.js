Pebble.addEventListener('showConfiguration', function(e) {
  Pebble.openURL('http://sendow.bitbucket.org/pebble/fuzzy-time/config.html');
});

Pebble.addEventListener('webviewclosed', function(e) {
  if (e.response) {
    var config_data = JSON.parse(decodeURIComponent(e.response));
    console.log('Config window returned: ', JSON.stringify(config_data));

    var dict = {
      'KEY_TEXT_ALIGN': config_data.textAlign
    };

    Pebble.sendAppMessage(dict, function(){
      console.log('Sent config data to Pebble');  
    }, function() {
      console.log('Failed to send config data!');
    });
  }
});
