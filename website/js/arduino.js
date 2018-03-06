function onArduinoEventsLoaded(response, status) {
    var parsedData = JSON.parse(response);
    var lastEvent = parsedData[0];
    var $alert = $('.js--arduino-alert');
    
    $alert.hide();
    
    if(!lastEvent){
        return;
    }
    
    $alert
        .show()
        .text('Last Arduino Event: ' + lastEvent.event + ' (' + (new Date(lastEvent.timestamp * 1)).toISOString() + ')');
}

$(function(){
    $.ajax('/cage/php/arduino_events.php', {
        success: onArduinoEventsLoaded,
        error: console.error.bind(console)
    });
});
