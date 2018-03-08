// col renderer
function renderColDate(timestamp, type) {
    if(type !== 'display'){
        return timestamp;
    }
    
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

function renderColResult(result, type){
    if(type !== 'display'){
        return result;
    }
    var possibleResults = {
        0: 'Success',
        1: 'Success but too slow',
        2: 'Aborted by mouse',
        3: 'Licked before airpuff',
        4: 'Licked between trials',
        5: 'Wrong licking port (left)',
        6: 'Wrong licking port (right)',
        7: 'Wrong licking port (middle)',
        8: 'Mouse did nothing'
    };
    if(!possibleResults[result]){
        return 'Unknown Result State (' + result + ')';
    }
    return possibleResults[result];
}

function renderColTimespan(ms, type){
    if(type !== 'display'){
        return ms;
    }
    
    return ms + 'ms';
}

function renderColChoice(result, type){
    if(type !== 'display'){
        return result;
    }
    return [null, 'Left', 'Right', 'Center'][result];
}

// select plugin
$(document).on( 'init.dt', function(e, settings){
    var api = $.fn.DataTable.Api(settings);
    var opts = api.init();
    var $wrapper = $(settings.nTableWrapper);
    var defaults = {
        prefix: '',
        postfix: '',
        multiple: false
    };
    
    opts.columns.forEach(function(col, i){
        if(!col.s2filter){
            return;
        }
        
        var s2opts = $.extend({}, defaults, col.s2filter);
        
        var data = api.column(i).cache('search').sort().unique().map(function(content){
            var text = content;
            if(typeof col.render === 'function'){
                text = col.render(text, 'display', null);
            }
            if(col.render && typeof col.render.display === 'function'){
                text = col.render.display(text);
            }
            
            return {
                id: content,
                text: s2opts.prefix + text + s2opts.postfix,
                selected: false
            };
        });
        
        var $select = $('<select>');
        $wrapper.prepend($select);
        
        $select.on('change', function(){
            var val = $select.val();
            var searchRegex = '';
            if(val && !Array.isArray(val)){
                val = [val];
            }
            
            if(val){
                searchRegex = '^(' + val.map(function(s){ return RegExp.escape(s) }).join('|') +')$';
            }
            api.column(i).search(searchRegex, true).draw();
        });
        
        if(!s2opts.multiple){
            $select.append('<option>');
        }
        
        $select.select2({
            placeholder: col.title,
            data: data,
            multiple: s2opts.multiple,
            width: '300px',
            allowClear: true
        });
    });
});


// reload button
$.fn.dataTable.ext.buttons.reload = {
    text: 'Reload',
    action: function ( e, dt, node, config ) {
        dt.ajax.reload();
    }
};
