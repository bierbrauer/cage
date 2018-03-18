var webcamIsRunning;

function toggleLiveStream(showStream){
    var $webcam = $('.js--webcam');
    
    $webcam.toggle(showStream);
    
    if(showStream){
        $webcam.attr('src',  getLivestreamUrl());
    } else {
        $webcam.removeAttr('src');
    }
}

function getLivestreamUrl(){
    var origin = window.location.protocol + '//' + window.location.host + ':8080';
    if(window.location.host.indexOf('localtunnel.me') !== -1){
        origin = 'https://camaglarkum.localtunnel.me';
    }
    return (origin + '/?action=stream&random=' + Math.random());
}

function webCamAction(action, cb, timeout){
    timeout = timeout || 0;
    $.getJSON('/cage/php/cam.php?action=' + action, function(res){
        setTimeout(function(){
            cb(res);
        }, timeout);
    });
}

function toggleWebcam(doStart){
    var action = 'status';
    var $button = $('.js--webcam-status');
    if(typeof doStart !== 'undefined'){
        action = doStart ? 'start' : 'stop';
    }
    
    $button
        .attr('disabled', true)
        .text('loading');
    
    webCamAction(action, function(status){
        webcamIsRunning = status.status === 'running';
        toggleLiveStream(webcamIsRunning);
        $button
            .attr('disabled', false)
            .text(webcamIsRunning ? 'Stop webcam' : 'Start webcam');
    }, webcamIsRunning ? 0 : 500);
}

$(function(){
    toggleWebcam();
    $('.js--webcam-status').attr('disabled', true).on('click', function(e){
        e.preventDefault();
        toggleWebcam(!webcamIsRunning);
    });
});
