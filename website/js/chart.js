var datasetColors = [
	'#537bc4',
	'#f67019',
	'#acc236',
	'#166a8f',
	'#00a950',
	'#58595b',
    '#4dc9f6',
	'#8549ba',
	'#f53794',
];

function getColorByIndex(i){
    return datasetColors[i % datasetColors.length];
}

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

function groupDataByMiceAndProgram(data){
    return groupDataBy(data, function(entry){
        return 'c:' + entry.cage + ';m:' + entry.mouse + entry.program;
    });
}

function groupDataByDay(data){
    return groupDataBy(data.sort(function(a, b){
        return new Date(a.trialStart * 1) - new Date(b.trialStart * 1);
    }), function(entry){
        return (new Date(entry.trialStart * 1)).toDateString();
    });
}

function getDataByMiceByDay(data){
    var dataByMice = groupDataByMice(data);
    return dataByMice.map(function(mice){
        return groupDataByDay(mice);
    });
}

function groupDataByMice(data){
    return groupDataBy(data, function(entry){
        return 'c:' + entry.cage + ';m:' + entry.mouse;
    });
}

function getDatasetPerformanceByMiceByDay(data){
    var groupedData = getDataByMiceByDay(data);
    
    return groupedData.map(function(mouse, i){
        var color = getColorByIndex(i);
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
                y: Math.round(goodTrials.length / trials.length * 10000) / 100,
                count: trials.length
            });
        });
        
        return dataset;
    });
}

function getDatasetPerformanceByMiceByTrial(data){
    var groupedData = groupDataByMiceAndProgram(data);
    
    return groupedData.map(function(trials, i){
        var color = getColorByIndex(i);
        var dataset = {
            label: 'Mouse ' + trials[0].mouse + ' Program ' + trials[0].program,
            backgroundColor: Chart.helpers.color(color).alpha(0.5).rgbString(),
            borderColor: color,
            data: []
        };
        
        trials = trials.sort(function(a, b){
            return a.trialStart - b.trialStart;
        });
        
        var previousTrials = [];
        var previousGoodTrials = [];
        
        trials.forEach(function(trial, i){
            if(trial.result === '0'){
                previousGoodTrials.push(trial);
            }
            previousTrials.push(trial);
            
            dataset.data.push({
                x: i + 1,
                y: previousGoodTrials.length / previousTrials.length * 100
            });
        });
        
        return dataset;
    });
}

function timeToDay(time){
    return new Date(time.getFullYear(), time.getMonth(), time.getDate());
}

function getDatasetCountByMiceByDay(data){
    var groupedData = getDataByMiceByDay(data);
    
    var DAYS_TO_BLOCK = 3;
    
    return groupedData.map(function(days, i){
        var color = getColorByIndex(i);
        var dataset = {
            label: 'Mouse ' + days[0][0].mouse,
            backgroundColor: Chart.helpers.color(color).alpha(0.5).rgbString(),
            data: [],
            pointBackgroundColor: [],
        };
        
        var lowestTime = new Date(days[0][0].trialStart * 1);
        var highestTime = new Date(days[days.length - 1][0].trialStart * 1);
        var lowestDay = timeToDay(lowestTime);
        var highestDay = timeToDay(highestTime);
        var endDay = timeToDay(new Date(highestTime * 1 + DAYS_TO_BLOCK * 86400000));
        var today = timeToDay(new Date());
        
        // in case of manipulated data
        var lastDate = today > highestDay ? today : highestDay;
        
        var currentDay = new Date(lowestDay);
        var dayRange = [];
        while(currentDay <= lastDate){
            dayRange.push(new Date(currentDay));
            currentDay.setDate(currentDay.getDate() + 1);
        }
        
        dayRange.forEach(function(day, i){
            if(i >= DAYS_TO_BLOCK && day > endDay){
                return;
            }
            
            var trialsForDay = days.filter(function(dayData){
                var trialDay = timeToDay(new Date(dayData[0].trialStart * 1));
                return trialDay * 1 === day * 1;
            })[0] || [];
            
            var ccolor = Chart.helpers.color(color).alpha(1).rgbString();
            if(trialsForDay.length === 0){
                ccolor = '#ff0000';
            }
            
            dataset.data.push({
                x: day,
                y: trialsForDay.length,
            });
            
            dataset.pointBackgroundColor.push(ccolor);
        });
        
        return dataset;
    });
}

var drawChart = (function(){
    var lastChart = null;
    
    return function drawChart(opts){
        if(lastChart){
            lastChart.destroy();
        }
        
        opts.options.maintainAspectRatio = false;
        
        var ctx = $('.js--graph').get(0).getContext('2d');
        lastChart = new Chart(ctx, opts);
        return lastChart;
    }
})();

function renderPerformanceDayChart(selectedData){
     var datasets = getDatasetPerformanceByMiceByDay(selectedData);
     
     drawChart({
        type: 'bar',
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
                    },
                    barPercentage: 1,
                    categoryPercentage: 0.9,
                    distribution: 'series'
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
                    },
                    gridLines: {
                        color: [null,null,null,null,null,'#000'],
                        lineWidth: [1, 1, 1, 1, 1, 2]
                    }
                }, {
                    position: 'right',
                    ticks: {
                        suggestedMin: 0,
                        suggestedMax: 100,
                        callback: function(value){
                            return value + '%';
                        }
                    }
                }]
            },
            tooltips: {
                callbacks: {
                    afterLabel: function(item, data){
                        var count = data.datasets[item.datasetIndex].data[item.index].count;
                        return 'Trials: ' + count;
                    }
                }
            }
        }
    });
}

function renderPerformanceTrialChart(selectedData){
     var datasets = getDatasetPerformanceByMiceByTrial(selectedData);
     
     drawChart({
        type: 'line',
        data: {
            datasets: datasets
        },
        options: {
            scales: {
                xAxes: [{
                    type: 'linear',
                    scaleLabel: {
                        display: true,
                        labelString: 'Trial'
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
                    },
                    gridLines: {
                        color: [null,null,null,null,null,'#000'],
                        lineWidth: [1, 1, 1, 1, 1, 2]
                    }
                }, {
                    position: 'right',
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

function renderCountDayChart(selectedData){
     var datasets = getDatasetCountByMiceByDay(selectedData);
     
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
                        labelString: 'Trials'
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
        'performance-trial': renderPerformanceTrialChart,
        'count-day': renderCountDayChart
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
    
    $('#trial_data').on('xhr.dt', $.debounce(250, function(e, dataTableSettings){
        dts = dataTableSettings;
        var searchId = getSearchId();
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

