<?php

$action = $_GET['action'];

if($action === 'start'){
    exec('sudo -u cage /etc/init.d/mjpg-streamer start 2>&1');
} elseif($action === 'stop'){
    exec('sudo -u cage /etc/init.d/mjpg-streamer stop 2>&1');
}

echo json_encode([
    'status' => exec('sudo -u cage /etc/init.d/mjpg-streamer status')
]);
