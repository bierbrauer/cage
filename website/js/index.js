window.addEventListener('load', start);

function parseDate(timestamp) {
    var date = new Date(Number(timestamp));

    var month = (date.getMonth() + 1);
    var day = date.getDate();
    var hours = date.getHours();
    var minutes = date.getMinutes();
    var seconds = date.getSeconds();
    var milliseconds = date.getMilliseconds();

    month = month < 10 ? '0' + month : month;
    day = day < 10 ? '0' + day : day;
    hours = hours < 10 ? '0' + hours : hours;
    minutes = minutes < 10 ? '0' + minutes : minutes;
    seconds = seconds < 10 ? '0' + seconds : seconds;
    
    if (milliseconds < 10) { milliseconds = '00' + milliseconds; }
    else if (milliseconds < 100) { milliseconds = '0' + milliseconds; }

    return date.getFullYear() + '-' +
        month + '-' +
        day + ' ' +
        hours + ':' + 
        minutes + ':' + 
        seconds + "." +
        milliseconds;
}

function onDataLoaded(response, status) {
    console.info('Data loaded: ' + status);
    
    var parsedData = JSON.parse(response);
    var tableData = [];

    parsedData.forEach(function(element, index) {
	tableData[index] = [];

        Object.keys(element).forEach(function(key, eIndex) {
            var value = element[key];

	    if (key === 'trialStart' || key === 'trialEnd') {
                value = parseDate(value);
            }

            tableData[index][eIndex] = value;
        });
    });

    $('#trial_data').DataTable({
        data: tableData,
        columns: [
            { title: "ID" },
            { title: "Cage" },
            { title: "Program" },
            { title: "Mouse" },
            { title: "Trial" },
            { title: "Trial Start" },
            { title: "Trial End" },
            { title: "Duration (ms)" },
            { title: "Result" },
            { title: "Wait4AP (ms)" },
            { title: "Waited (ms)" },
            { title: "Iteration" }
        ]
    });
}

function onArduinoEventsLoaded(response, status) {
    console.info('Data loaded: ' + status);
    
    var parsedData = JSON.parse(response);
    var tableData = [];

    parsedData.forEach(function(element, index) {
	tableData[index] = [];

        Object.keys(element).forEach(function(key, eIndex) {
            var value = element[key];

	    if (key === 'timestamp') {
                value = parseDate(value);
            }

            tableData[index][eIndex] = value;
        });
    });

    $('#arduino_events').DataTable({
        data: tableData,
        columns: [
            { title: "ID" },
            { title: "SerialNumber" },
            { title: "SerialPort" },
            { title: "Timestamp" },
            { title: "Event" }
        ]
    });
}

function onError() {
    console.error('Error fetching data!');
}

function start() {
    console.log('Start loading data...');

    $.ajax('/cage/php/sql.php', {
        success: onDataLoaded,
        error: onError
    });

    $.ajax('/cage/php/arduino_events.php', {
        success: onArduinoEventsLoaded,
        error: onError
    });
}