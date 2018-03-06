var datasetColors = [
	'#f67019',
	'#f53794',
	'#537bc4',
	'#acc236',
	'#166a8f',
	'#00a950',
	'#58595b',
    '#4dc9f6',
	'#8549ba'
];

function groupDataBy(data, getIdCb){
    var dataByGroup = {};
    data.forEach(function(entry){
        var id = getIdCb(entry);
        
        var data = dataByGroup[id];
        if(!data){
            data = []
            dataByGroup[id] = data;
        }
        
        data.push(entry);
    });
    return Object.values(dataByGroup);
}

function groupDataByMice(data){
    return groupDataBy(data, function(entry){
        return 'c:' + entry.cage + ';m:' + entry.mouse;
    });
}

function groupDataByDay(data){
    return groupDataBy(data, function(entry){
        return (new Date(entry.trialStart * 1)).toDateString();
    });
}

function groupDataByTrial(data){
    return groupDataBy(data, function(entry){
        return 't:' + entry.trial;
    });
}

function getDataByMiceByDay(data){
    var dataByMice = groupDataByMice(data);
    return dataByMice.map(function(mice){
        return groupDataByDay(mice);
    });
}

function getDatasetByMiceByDay(data){
    var groupedData = getDataByMiceByDay(data);
    
    return groupedData.map(function(mouse, i){
        var color = datasetColors[datasetColors.length % i];
        var dataset = {
            label: 'Mouse ' + mouse[0][0].mouse,
            backgroundColor: Chart.helpers.color(color).alpha(0.5).rgbString(),
            borderColor: color,
            data: []
        };
        
        mouse.map(function(trials){
            var goodTrials = trials.filter(function(trial){
                return trial.result === '0';
            });
            var dayDate = new Date((new Date(trials[0].trialStart * 1)).toDateString())
            dataset.data.push({
                x: dayDate,
                y: Math.round(goodTrials.length / trials.length * 10000) / 100
            });
        });
        
        return dataset;
    });
}

function getDatasetByMiceByTrial(data){
    var groupedData = groupDataByMice(data);
    
    return groupedData.map(function(trials, i){
        var color = datasetColors[datasetColors.length % i];
        var dataset = {
            label: 'Mouse ' + trials[0].mouse,
            backgroundColor: Chart.helpers.color(color).alpha(0.5).rgbString(),
            borderColor: color,
            data: []
        };
        
        trials = trials.sort(function(a, b){
            return a.id - b.id;
        });
        
        var previousTrials = [];
        var previousGoodTrials = [];
        
        trials.forEach(function(trial, i){
            if(trial.result === '0'){
                previousGoodTrials.push(trial);
            }
            previousTrials.push(trial);
            
            dataset.data.push({
                x: i,
                y: previousGoodTrials.length / previousTrials.length * 100
            });
        });
        
        return dataset;
    });
}

function drawChart(opts){
    var ctx = $('.js--graph').get(0).getContext('2d');
    return new Chart(ctx, opts);
}

function renderPerformanceDayChart(selectedData){
     var datasets = getDatasetByMiceByDay(selectedData);
     
     drawChart({
        type: 'line',
        data: {
            datasets: datasets
        },
        options: {
            scales: {
                xAxes: [{
                    type: "time",
                    time: {
                        tooltipFormat: 'll',
                        unit: 'day',
                        displayFormats: {
                            day: 'll'
                        }
                    },
                    scaleLabel: {
                        display: true,
                        labelString: 'Day'
                    },
                    ticks: {
                        source: 'data'
                    }
                }],
                yAxes: [{
                    scaleLabel: {
                        display: true,
                        labelString: 'Success'
                    },
                    ticks: {
                        suggestedMin: 0,
                        suggestedMax: 100,
                        callback: function(value){
                            return value + '%';
                        }
                    }
                }]
            }
        }
    });
}

function renderPerformanceTrialChart(selectedData){
     var datasets = getDatasetByMiceByTrial(selectedData);
     
     drawChart({
        type: 'line',
        data: {
            datasets: datasets
        },
        options: {
            scales: {
                xAxes: [{
                    scaleLabel: {
                        display: true,
                        labelString: 'Trial'
                    }
                }],
                yAxes: [{
                    scaleLabel: {
                        display: true,
                        labelString: 'Success'
                    },
                    ticks: {
                        suggestedMin: 0,
                        suggestedMax: 100,
                        callback: function(value){
                            return value + '%';
                        }
                    }
                }]
            }
        }
    });
}

function renderSelectedChart(selectedData){
    var $select = $('.js--graph-type');
    var charts = {
        'performance-day': renderPerformanceDayChart,
        'performance-trial': renderPerformanceTrialChart
    };
    charts[$select.val()](selectedData);
}

function listenOnSearchChangeGraphSelect(cb){
    var getSearchId = function(){
        return dataTable.search() + ',' + dataTable.columns().search().join(',');
    };
    var dts = null;
    var dataTable = $('#trial_data').DataTable();
    var lastSearchId = null;
    
    $('#trial_data').on('search.dt init.dt', $.debounce(250, function(e, dataTableSettings){
        dts = dataTableSettings;
        var searchId = getSearchId();
        if(searchId === lastSearchId){
            return;
        }
        cb(dts);
        lastSearchId = searchId;
    }));
    
    $('.js--graph-type').on('change', function(){
        cb(dts);
    });
}

$(function(){
    listenOnSearchChangeGraphSelect(function(dataTableSettings){
        var selectedIndices = dataTableSettings.aiDisplay;
        var selectedData = selectedIndices.map(function(index){
            return dataTableSettings.aoData[index]._aData;
        });
        
        renderSelectedChart(selectedData);
    });
});

