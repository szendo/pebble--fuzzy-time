Pebble.addEventListener('showConfiguration', function(e) {
  Pebble.openURL('https://aveao.github.io/Pebble-App-Config-Page-Backup/archive/sendow.github.io/pebble/fuzzy-time/config/config_55855cb56b94559b530000c5_basalt.html');
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
