$(function(){
    var dataTable = $('#trial_data').DataTable({
        dom: 'Bltpri',
        ajax: '/cage/php/sql.php',
        order: [[6, 'desc']],
        buttons: [
            'reload', 'csv'
        ],
        columns: [
            { 
                data: "id",
                title: "ID",
                render: $.fn.dataTable.render.text()
            },
            {
                data: "cage",
                title: "Cage",
                render: $.fn.dataTable.render.text()
            },
            {
                data: "program",
                title: "Program",
                s2filter: {
                    prefix: 'Program  '
                },
                render: $.fn.dataTable.render.text()
            },
            {
                data: "mouse",
                title: "Mouse",
                s2filter: {
                    multiple: true,
                    prefix: 'Mouse '
                },
                render: $.fn.dataTable.render.text(),
            },
            {
                data: 'options',
                title: 'Options',
                render: $.fn.dataTable.render.text()
            },
            {
                data: "trial",
                title: "Demanded",
                s2filter: {
                    prefix: 'Demanded '
                },
                render: renderColChoice
            },
            {
                data: "trialStart",
                title: "Trial Start",
                render: renderColDate
            },
            {
                data: "trialEnd",
                title: "Trial End",
                render: renderColDate
            },
            {
                data: "duration",
                title: "Duration",
                render: renderColTimespan
            },
            {
                data: "result",
                title: "Result",
                s2filter: {
                    multiple: true
                },
                render: renderColResult
            },
            {
                data: "wait4AP",
                title: "Wait4AP",
                render: renderColTimespan
            },
            {
                data: "waited",
                title: "Waited",
                render: renderColTimespan 
            },
            {
                data: "iteration",
                title: "Iteration",
                render: $.fn.dataTable.render.text() 
            }
        ]
    });

    $('.js--data-search').on('keyup', function(){
        var searchString = $(this).val();
        dataTable.search(searchString).draw();
    });
});
