var interval_position = -1;
const cmd_SetPosition000='G92 X0 Y0 Z0';
const cmd_SetPositionZ0='G92 Z0';
const cmd_AutoHome='G28';
const cmd_AutoHomeXY='G28 X Y';
const cmd_AutoHomeX='G28 X';
const cmd_AutoHomeY='G28 Y';
const cmd_AutoHomeZ='G28 Z';
const cmd_DisableSteppers ='M84';
const cmd_AbsolutePositioning ='G90';
const cmd_RelativePositioning ='G91';
const cmd_ReportSDPrintStatus ='M27';
const cmd_LinearMoveIdle='G0';
const cmd_GetCurrentPosition='M114';
const cmd_SetLCDMessage='M117';
const cmd_cancelShutdown='M85 S0';

function init_controls_panel() {    
    document.getElementById('control_xy_velocity').value =  config.feedrate.move_xy;
    document.getElementById('control_z_velocity').value =  config.feedrate.move_z;
    marlin_addCommand(cmd_GetCurrentPosition);
    marlin_addCommand(cmd_ReportSDPrintStatus);
}


function controls_UpdateStatus(status){
    if((typeof status.position)!='undefined'){
        if((typeof status.position.X)!='undefined')
            document.getElementById('control_x_position').innerHTML = status.position.X;
        if((typeof status.position.Y)!='undefined')
            document.getElementById('control_y_position').innerHTML = status.position.Y;
        if((typeof status.position.Z)!='undefined'){
            document.getElementById('control_z_position').innerHTML = status.position.Z;
        }
    }
    if((typeof status.SDPrintStatus)!='undefined'){
        if(status.SDPrintStatus.state){
            $('#id_printProgressVal').html(status.SDPrintStatus.pos+"/"+status.SDPrintStatus.total);
            $('#id_printProgress').width(parseInt(status.SDPrintStatus.pos)*100/parseInt(status.SDPrintStatus.total)+"%")
        }else{
            $('#id_printProgressVal').html("no SD printing");
        }
    }
}

function loadmacrolist(macrosName) {    
    var url = "/config/"+macrosName;
    SendGetHttp(url, Macro_build_list, processMacroGetFailed);
}

function Macro_build_list(response_text) {
    var macros_list=[];
    try {        
        macros_list = JSON.parse(response_text);
    } catch (e) {
        console.error("Parsing error:", e);
    }
    if((typeof macros_list.items)!='undefined' && (macros_list.items.length)){
        var content="";
        macros_list.items.forEach(function(item){
            content += control_build_macro_button(item);
        })
        document.getElementById('Macro_list').innerHTML = content;
        document.getElementById('MacroPanel').hidden=false;
    }
}

function processMacroGetFailed(errorcode, response) {
    console.log("Error " + errorcode + " : " + response);
    Macro_build_list("");
}

function get_Position() {
    marlin_addCommand(cmd_GetCurrentPosition);
}

function controls_GotoZero(fZZero) {
    var zHop = $('#id_ProbeZhop').val();
    var feedrateZ = $('#control_z_velocity').val();
    var feedrateXY = $('#control_xy_velocity').val();
    var command =cmd_RelativePositioning+ "\nG0"+" F" + feedrateZ+" Z" + zHop;
    command+="\nG90\nG0"+" F" + feedrateXY+" X0 Y0";
    if(fZZero){
        command+="\nG0"+" F"+feedrateZ+" Z0"
    }
    command+="\n"+cmd_GetCurrentPosition;
    marlin_addCommand(command);
}

function control_build_macro_button(item) {
    var content = "";
    content += "<button class='btn btn-outline-primary' type='text' ";
    content += "onclick='marlin_addCommand(\""+item.cmd+"\")'";
    content += "><span style='position:relative; top:3px;'>";     
    content += "</span>";
    if (item.name.length > 0) {
        content += "&nbsp;";
    }
    content += item.name;    
    content += "</button>";
    return content;
}

function controls_ProbeTargetMultiple(fActive) {
    if(fActive){
        url="/probe?mode=multiple";
        url+="&levelDelta="+parseFloat($('#id_levelDelta').val());
        url+="&feedRateProbe="+parseInt($('#id_ProbeFeed').val());
        url+="&doubleTouch=0";
        SendGetHttp(url,on_httpStatusResponce,engraver_ResultError);
    }else{
        engraver_probeStop();
    }
}

function controls_ProbeTargetMultiple_callback_hit(response){
    if(response.indexOf("\nError:Failed to reach target")==-1){ //hit
        const feedRateProbe=parseInt(document.getElementById('id_ProbeFeed').value)/2;
        clear_command_list();
        controls_gotoZHop(config.probe.double.hop);
        var command = "G38.2 F" +feedRateProbe+ " Z" + config.probe.double.distance;
        command+="\n"+cmd_GetCurrentPosition;
        marlin_addCommand(command);   
    }
}

function controls_ProbeTarget(probeDistance_) {
    clear_command_list();
    var probeDistance=parseFloat(probeDistance_);
    console.log(probeDistance);
    const feedRateZ = parseInt(document.getElementById('control_z_velocity').value);
    const feedRateProbe=parseInt(document.getElementById('id_ProbeFeed').value);
    marlin_addCommand(cmd_RelativePositioning);
    if(-20>probeDistance){
        var command = "G38.2 F" +feedRateZ+ " Z" + (probeDistance+20);
        marlin_addCommand(command,controls_ProbeTargetMultiple_callback_hit);
        probeDistance=-20;
    }
    if(-5>probeDistance){
        var command = "G38.2 F" +feedRateProbe*2+ " Z" + (probeDistance+5);
        marlin_addCommand(command,controls_ProbeTargetMultiple_callback_hit);
        probeDistance=-5;
    }
    var command = "G38.2 F" +feedRateProbe+ " Z" + probeDistance;
    marlin_addCommand(command,controls_ProbeTargetMultiple_callback_hit);
    marlin_addCommand(cmd_GetCurrentPosition); //will run if no hit
}

function controls_gotoToPos(posX,posY,posZ){
    var command = cmd_AbsolutePositioning+"\n";
    if((typeof posZ)!='undefined'){
        var feedrateZ = parseInt(document.getElementById('control_z_velocity').value);
        command += "G0"+" F" + feedrateZ+" Z" + posZ+"\n";         
    }
    var feedrate = parseInt(document.getElementById('control_xy_velocity').value);
    command+= "G0"+" F" + feedrate+" X" + posX + " Y"+posY;
    command+="\nM114"
    marlin_addCommand(command);    
}

function controls_gotoZHop(deltaZ){
    console.log(deltaZ);
    var feedrate = parseInt(document.getElementById('control_z_velocity').value);
    var command = cmd_RelativePositioning+"\nG0"+" F" + feedrate+" Z" + deltaZ;
    command+="\n"+cmd_GetCurrentPosition;
    marlin_addCommand(command);    
}

function SDPrintStatus_setAutoReport(fActive){
    var command = cmd_ReportSDPrintStatus+' ';
    if(fActive){
        command+='S10';
    }else{
        command+='S0'; //off
    }
    marlin_addCommand(command);  
}

function marlin_modalMoveAxis(axis){
    console.log(axis);
    var index,val;
    if(axis==='X'){
        index=0;
        val=$('#control_x_position').html();
    }else if(axis==='Y'){
        index=1;
        val=$('#control_y_position').html();
    }else if(axis==='Z'){
        index=2;
        val=$('#control_z_position').html();
    }else{
        console.error("unknow axis");
        return; 
    }
    $('#id_ModalMoveAxisName').html(axis);
    $('#modal_rangePos').attr('min',config.bed.min[index]);
    $('#modal_rangePos').attr('max',config.bed.min[index]+config.bed.size[index]);
    $('#modal_rangePos').val(parseInt(val));
    $('#id_ModalMoveAxis').modal('toggle');
}

function marlin_modalMoveAxisPos(axis,value){
    console.log(axis,value);
    var feedrate = document.getElementById('control_xy_velocity').value;
    if(axis==='Z'){
        feedrate = document.getElementById('control_z_velocity').value;
    }
    var command =cmd_AbsolutePositioning;
    command+='\n'+cmd_LinearMoveIdle+' F'+feedrate+' '+axis+value;
    command+='\n'+cmd_GetCurrentPosition;
    marlin_addCommand(command);
}

function marlin_modalMoveAxisStep(axis,step){
    console.log(axis,step);
    var feedrate = document.getElementById('control_xy_velocity').value;
    if(axis==='Z'){
        feedrate = document.getElementById('control_z_velocity').value;
    }
    var command =cmd_RelativePositioning;
    command+='\n'+cmd_LinearMoveIdle+' F'+feedrate+' '+axis+step;
    command+='\n'+cmd_GetCurrentPosition;
    marlin_addCommand(command);
}
