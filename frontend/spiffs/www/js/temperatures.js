var graph_started = false;

var smoothieextuder = new SmoothieChart({
    millisPerPixel: 200,
    maxValueScale: 1.1,
    minValueScale: 1.1,
    enableDpiScaling: false,
    interpolation: 'linear',
    grid: {
        fillStyle: '#ffffff',
        strokeStyle: 'rgba(128,128,128,0.5)',
        verticalSections: 5.,
        millisPerLine: 0,
        borderVisible: false
    },
    labels: {
        fillStyle: '#000000',
        precision: 1
    }
});
var smoothiebed = new SmoothieChart({
    millisPerPixel: 200,
    interpolation: 'linear',
    maxValueScale: 1.1,
    minValueScale: 1.1,
    enableDpiScaling: false,
    grid: {
        fillStyle: '#ffffff',
        strokeStyle: 'rgba(128,128,128,0.5)',
        verticalSections: 5.,
        millisPerLine: 0,
        borderVisible: false
    },
    labels: {
        fillStyle: '#000000',
        precision: 1
    }
});
var extruder_0_line = new TimeSeries();
var bed_line = new TimeSeries();

function init_temperature_panel() {

    document.getElementById('temperature_heatOff_T0').innerHTML = get_icon_svg("off", "1.3em", "1.2em");
    document.getElementById('temperature_heatOff_Bed').innerHTML = get_icon_svg("off", "1.3em", "1.2em");
    document.getElementById('temperature_heatSet_T0').innerHTML = get_icon_svg("ok", "1.3em", "1.2em");
    document.getElementById('temperature_heatSet_Bed').innerHTML = get_icon_svg("ok", "1.3em", "1.2em");

    smoothiebed.addTimeSeries(bed_line, {
        lineWidth: 1,
        strokeStyle: '#808080',
        fillStyle: 'rgba(128,128,128,0.3)'
    });
    smoothieextuder.addTimeSeries(extruder_0_line, {
        lineWidth: 1,
        strokeStyle: '#ff8080',
        fillStyle: 'rgba(255,128,128,0.3)'
    });
    smoothieextuder.streamTo(document.getElementById("extruderTempgraph"), 3000 /*delay*/ );
    smoothiebed.streamTo(document.getElementById("bedTempgraph"), 3000 /*delay*/ );

    start_graph_output();             
    get_Temperatures();
}

function start_graph_output() {
    smoothieextuder.start();
    smoothiebed.start();
    graph_started = true;
}

function stop_graph_output() {
    smoothieextuder.stop();
    smoothiebed.stop();
    graph_started = false;
}

function get_Temperatures() {
    marlin_addCommand("M105");    
}

function temperatures_UpdateStatus(status){
     if((typeof status.temperature)!='undefined'){
        var timedata = new Date().getTime();
        if((typeof status.temperature.T)!='undefined'){
            extruder_0_line.append(timedata, status.temperature.T.current);
            document.getElementById('heaterT0DisplayTemp').innerHTML = temperatures_create_item(status.temperature.T);
            document.getElementById('heaterT0TargetTemp_anime').style.visibility = (status.temperature.T.target>0)?"visible": "hidden";
        }
        if((typeof status.temperature.B)!='undefined'){
            bed_line.append(timedata, status.temperature.B.current);
            document.getElementById('bedDisplayTemp').innerHTML = temperatures_create_item(status.temperature.B)
            document.getElementById('bedTargetTemp_anime').style.visibility = (status.temperature.B.target>0)?"visible": "hidden";
        }
    }
}

function temperatures_create_item(data){
    var value = "<span>" + data.current + "</span>";
    value += "&nbsp;";
    value +="<span>|</span>&nbsp;<span>" + data.target + "</span>";
    return value
}


function temperature_heatOff(target) {
    switch (target) {
        case 'T0':
            document.getElementById('heaterT0SelectedTemp').value = 0;
            break;
        case 'bed':
            document.getElementById('bedSelectedTemp').value = 0;
            break;
    }
    var type = (target == 'bed') ? 140 : 104;
    var command = "M" + type + " S0";
    if (target != 'bed') {
        command += " " + target;
    }
    marlin_addCommand(command)
    get_Temperatures();    
}

function temperature_handleKeyUp(event, target) {
    if (event.keyCode == 13) {
        temperature_heatSet(target);
    }
    return true;
}

function temperature_heatSet(target) {
    var selectedTemp = 0;
    switch (target) {
        case 'T0':
            selectedTemp = parseInt(document.getElementById('heaterT0SelectedTemp').value);
            break;
        case 'bed':
            selectedTemp = parseInt(document.getElementById('bedSelectedTemp').value);
            break;
    }

    var type = (target == 'bed') ? 140 : 104;
    var command = "M" + type + " S" + selectedTemp;
    if (target != 'bed') {
        command += " " + target;
    }
    marlin_addCommand(command);
}

function temperatures_setAutoReport(stateOn){
    var command="M155 ";
    command+=(stateOn)? "S15" : "S0";
    marlin_addCommand(command);
    get_Temperatures();
}