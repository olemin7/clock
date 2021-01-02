function engraver_applyProbeAreaSize(response){
    var gcodeInfo;
    try {        
        gcodeInfo = JSON.parse(response);
        $('#id_ProbeAreaSizeX').val(Math.ceil(gcodeInfo.dimention.max.x));
        $('#id_ProbeAreaSizeY').val(Math.ceil(gcodeInfo.dimention.max.y));
    } catch (e) {
        console.error("Parsing error:", e);
    }
}

function engraver_ResultError(status,response){
    alert("Error:"+status+",text="+response);
}

function engraver_getGcodeInfo(filename,handler){
    if(filename!=""){
        var url="/gcodeInfo?file="+encodeURI(filename);
        SendGetHttp(url, handler,engraver_ResultError);
    }
}

function engraver_selectFile(filename){
    document.getElementById("id_fileSelected").value=filename;
    if(filename===""){
        document.getElementById("id_getSizeFromFile").classList.add("disabled");
        document.getElementById("id_levelMod").classList.add("disabled");
    }else{
        document.getElementById("id_getSizeFromFile").classList.remove("disabled");
        document.getElementById("id_levelMod").classList.remove("disabled");
    }
    
}

function init_engraver_panel(){
    engraver_selectFile("");
    $('#id_ProbeAreaSizeX').attr('max',config.bed.size[0]);
    $('#id_ProbeAreaSizeX').val(30);
    $('#id_ProbeAreaSizeY').attr('max',config.bed.size[1]);
    $('#id_ProbeAreaSizeY').val(30);
    $('#id_feedRateProbeXY').attr('max',config.feedrate.max_xy); 
    $('#id_feedRateProbeXY').val(config.probe.feedrate_xy); 
    $('#id_ProbeAreaGrid').val(config.probe.grid);
    $('#id_levelDelta').val(config.probe.area.distance);  
    $('#id_levelDelta').attr('min',config.probe.distance_min); 
    $('#id_levelDelta').attr('max',config.probe.distance_max); 
    $('#id_ToolChangeZ').val(config.engraver.ToolChangeZ);   
}

function engraver_probeArea(){
    url="/probe?mode=area";
    url+="&sizeX="+parseInt($('#id_ProbeAreaSizeX').val());
    url+="&sizeY="+parseInt($('#id_ProbeAreaSizeY').val());
    url+="&grid="+parseInt($('#id_ProbeAreaGrid').val());
    url+="&levelDelta="+parseFloat($('#id_levelDelta').val());
    url+="&feedRateXY="+parseInt($('#id_feedRateProbeXY').val());
    url+="&feedRateProbe="+parseInt($('#id_ProbeFeed').val());
    url+="&doubleTouch=0";
    SendGetHttp(url,on_httpStatusResponce,engraver_ResultError);
}

function engraver_probeStop(){
    url="/probe?mode=stop";
    SendGetHttp(url,on_httpStatusResponce,engraver_ResultError);
}

function engraver_GCodeLevelMod_resp(response){
    var status={};
    status.fsChanged=true;
    on_statusUpdate(status);

}
function engraver_GCodeLevelMod(filename){
    if(filename!=""){
        url="/levelmod?file="+encodeURI(filename);
        SendGetHttp(url,engraver_GCodeLevelMod_resp,engraver_ResultError);
    }
}

function engraver_probeGetResult(){
    url="/probe?mode=get";
    SendGetHttp(url,undefined ,engraver_ResultError);
}
