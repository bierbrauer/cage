var webcamIsRunning;
var webcamTimeoutId;

function imageLoop(){
    var $webcam = $('.js--webcam');
    var loadStart;
    var tick = function(){
        loadStart = Date.now();
        $webcam.attr('src',  getLivestreamUrl());
        scheduleTick();
    };
    
    var scheduleTick = function(){
        if(!webcamIsRunning){
            return;
        }
        $webcam.on('load', function(){
            var loadTime = Date.now() - loadStart;
            clearTimeout(webcamTimeoutId);
            if(loadTime < 1000 / 16){
                webcamTimeoutId = setTimeout(tick, 1000 / 16);
            } else {
                webcamTimeoutId = setTimeout(tick, 1);
            }
        });
    }
    
    tick();
}

function toggleLiveStream(showStream){
    var $webcam = $('.js--webcam');
    
    $webcam.toggle(showStream);
    
    if(showStream){
       imageLoop()
    } else {
        if(webcamTimeoutId){
            clearTimeout(webcamTimeoutId);
        }
        $webcam.removeAttr('src');
    }
}

function getLivestreamUrl(){
    var origin = window.location.protocol + '//' + window.location.host + ':8080';
    if(window.location.host.indexOf('localtunnel.me') !== -1){
        origin = '//camaglarkum.localtunnel.me';
    }
    if(window.location.host.indexOf('scn.cx') !== -1){
        origin = '//aglarkum-cam.scn.cx';
    }
    return (origin + '/?action=snapshot&random=' + Math.random());
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
