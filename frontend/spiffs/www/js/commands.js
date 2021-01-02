var CustomCommand_history = [];
var CustomCommand_history_index = -1;
var command_list ;
var is_processing_command;
var request_uart_buffer_timer;
const TIMEOUT_requesrCMDReply=300;
const TIMEOUT_requesrUart=1000;

function init_command_panel() {
    clear_command_list();
    process_command_list();  
}

function Monitor_output_callback(output){
	var status={};
    output.split("\n").forEach(function(line){
        const trimed=line.trimEnd();
        if(trimed.length){
            marlin_processPosition(trimed,status);
            process_Temperatures(trimed,status);
            marlin_SDPrintStatus(trimed,status);
            marlin_SDcontrolSetToMarlin(trimed,status);
        }
    });
    on_statusUpdate(status);
}

function process_Temperatures(line,status) {    
    var regex_temp = /(B|T(\d*)):\s*([+]?[0-9]*\.?[0-9]+)? (\/)([+]?[0-9]*\.?[0-9]+)?/gi;
    var result;
   
    while ((result = regex_temp.exec(line)) !== null) {
        if((typeof status.temperature)=='undefined'){
            status.temperature={};
        }
        var tool = result[1];
        var value =0.00;
        var value2=0.00;
        if (!isNaN(parseFloat(result[3]))){   
            value = parseFloat(result[3]).toFixed(2);
        }
        if (!isNaN(parseFloat(result[5]))){
            value2 = parseFloat(result[5]).toFixed(2);
        }
        status.temperature[tool]={
            current:value,
            target:value2
        };
    }
}

function marlin_processPosition(line,status) {
    var reg=/^X:-?\d+\.\d+\sY:-?\d+\.\d+\sZ:-?\d+\.\d+/;
    if(reg.test(line)){
        if((typeof status.position)=='undefined'){
            status.position={};
        }
        var reg_f=/(X|Y|Z):(-?\d+\.\d+)/gi;
        while ((coord = reg_f.exec(line)) !== null){
        	status.position[coord[1]]=parseFloat(coord[2]).toFixed(2);   
        }
    };
}

function marlin_SDPrintStatus(line,status) {
    const NotSDprinting ="Not SD printing";
    if(line===NotSDprinting){
        if((typeof status.SDPrintStatus)=='undefined'){
            status.SDPrintStatus={};
        }
        status.SDPrintStatus.state=false;
        return;
    }
    var reg=/SD printing byte\s+\d+\/\d+/;
    if(reg.test(line)){
        if((typeof status.SDPrintStatus)=='undefined'){
            status.SDPrintStatus={};
        }
        status.SDPrintStatus.state=true;
        var reg_f=/\d+/g;
        var poss= [...line.matchAll(reg_f)];
        if(poss.length ==2){
            status.SDPrintStatus.pos=parseInt(poss[0]);
            status.SDPrintStatus.total=parseInt(poss[1]);
        }
    }
}

function marlin_SDcontrolSetToMarlin(line,status){
    const fl_setToMarlin = "// action:setStateSDcontrolMarlin";
    if(line===fl_setToMarlin){
        setStateSDcontrol();
    }
}

function power_off() {
    if (confirm('Target Power Off?')){
        clear_command_list();
        marlin_addCommand("M81"); //Power Off
    }
}

function emergency_off() {
    if (confirm('Emergency Target Power Off?')){
        clear_command_list();
        marlin_addCommand("M112"); //Emergency Stop neet to Enable EMERGENCY_PARSER
    }
}

function clear_command_list() {
    command_list = [];    
    is_processing_command = false;    
}

function marlin_addCommand(command_,callback) {    
    command_.split("\n").forEach(function(line){
        if(line!=""){
            var desc={
                cmd:line,
                handler:callback
            }
            command_list.push(desc);
        }
    })
    process_command_list();
}

function process_command_list() {
    clearTimeout(request_uart_buffer_timer);
    if(command_list.length){   
        if(!is_processing_command){
            var desc=command_list[0];
            command_list.shift(); 
            ProcessCommand(desc.cmd,desc.handler);
        }else{ 
            setTimeout(process_command_list,TIMEOUT_requesrCMDReply); //to recheck finish
        }        
    }else{     
        request_uart_buffer_timer=setTimeout(commands_uart_listener,TIMEOUT_requesrUart);
    }
}


var xmlhttp = new XMLHttpRequest();
var xml_cmd;
var command_wait_count;
var command_answer;
var command_callback;

function commands_uart_listener() {    
    if(command_list.length || is_processing_command){
        return;
    }
    is_processing_command=true;
    xmlhttp.onreadystatechange = function() {
        if (xmlhttp.readyState == 4) {
            if (xmlhttp.status == 200) {                                     
                Monitor_output_Update(xmlhttp.responseText);
            };
            is_processing_command=false;
            request_uart_buffer_timer=setTimeout(commands_uart_listener,TIMEOUT_requesrUart);
        }        
    }
    url="/command";
    xmlhttp.open("GET", url, true);
    xmlhttp.send(null);
}

function ProcessCommand(cmd,callback) {
    if(is_processing_command)return;           
    xml_cmd=cmd;   
    command_answer="";
    command_callback=callback;
    command_wait_count=100;//waiting count
    is_processing_command=true;
    xmlhttp.onreadystatechange = function() {
        if (xmlhttp.readyState == 4) {
            if (xmlhttp.status == 200) {    
                xmlhttp.responseText.split("\n").forEach(function(line){
                    if(is_processing_command){
                        command_answer+=line;
                        command_answer+="\n";  
                        if(line.startsWith("ok")){
                            setTimeout(process_command_list,100);//to next cmd
                            if((typeof command_callback)!='undefined'){
                                command_callback(command_answer);
                            }
                            is_processing_command=false;
                        }  
                    }
                });      
                if(is_processing_command){ //no "ok" yet
                    if(command_wait_count){
                        console.log("wait ok");
                        setTimeout(commands_ProcessCmdWait,TIMEOUT_requesrCMDReply);
                        command_wait_count--;
                    }else{
                        console.log("cmd timeout");
                        Monitor_comment("cmd timeout");
                        is_processing_command=false;
                        setTimeout(process_command_list,TIMEOUT_requesrCMDReply);//to next cmd
                    }
                }
            } else {                
				console.log("http Error " +xmlhttp.status);
				Monitor_comment("http Error ["+xml_cmd+"]"+xmlhttp.status+'\n');
                is_processing_command=false;
                setTimeout(process_command_list,TIMEOUT_requesrCMDReply);//to next cmd
            }
            Monitor_output_Update(xmlhttp.responseText);
        }        
    }

    cmd = encodeURI(cmd).replace("#", "%23");//because # is not encoded
    url="/command?commandText="+cmd;
    console.log("GET:" + url);
    xmlhttp.open("GET", url, true);
    xmlhttp.send(null);
}

function commands_ProcessCmdWait() {
    xmlhttp.open("GET", "/command", true);
    xmlhttp.send(null);
}

function Monitor_check_autoscroll() {
    if (document.getElementById('monitor_enable_autoscroll').checked == true){
    	document.getElementById('cmd_content').scrollTop = document.getElementById('cmd_content').scrollHeight;
    }
}

function Monitor_output_Clear() {
    var list=document.getElementById("cmd_content");
    while (list.hasChildNodes()) {  
      list.removeChild(list.firstChild);
    }
}

function Monitor_output_Update_by_line(output) {
    //Filter the output  
    if ((output.trim() === "\n") || (output.trim() === "\r") || (output.trim() === "\r\n") || (output.trim() === "")) return;
    if (output.startsWith("[#]")) {
        output = output.replace("[#]", "");
    }         
    output+="\n";
    if (!output.startsWith(";")) {//comment
	   Monitor_output_callback(output);
    }

    var cmd_content=document.getElementById("cmd_content");
    if(document.getElementById('id_GroupSimilar').checked == true &&
        cmd_content.childNodes.length && (output===cmd_content.childNodes[cmd_content.childNodes.length-1].nodeValue)){
        var b;
        if(1<cmd_content.childNodes.length){
            var pre=cmd_content.childNodes[cmd_content.childNodes.length-2];            
            if(pre.nodeType==Node.ELEMENT_NODE && pre.classList.contains("badge")){
                pre.textContent=parseInt(pre.textContent)+1;
                return;
            }
        }
        b=document.createElement('span');
        b.classList.add('badge');
        b.classList.add('badge-primary');        
        b.textContent ="2";        
        document.getElementById("cmd_content").insertBefore(b,cmd_content.lastChild);
        return;
    }

    var n=document.createTextNode(output);
    document.getElementById("cmd_content").appendChild(n);
    Monitor_check_autoscroll();
}
function Monitor_output_Update(output){ //few lines will be added by one
    output.split("\n").forEach(function(line){
        Monitor_output_Update_by_line(line);
    })
}

function Monitor_comment(comment){ //few lines will be added by one
    comment.split("\n").forEach(function(line){
        if(line.length){
            Monitor_output_Update_by_line(";"+line);
        }
    })
}

function SendCustomCommand() {
    var cmd = document.getElementById("custom_cmd_txt").value;
    document.getElementById("custom_cmd_txt").value = "";    
    cmd = cmd.trim();
    if (cmd.trim().length == 0) return;
    
    if (document.getElementById('monitor_upCase').checked == true){
        cmd =cmd.toUpperCase();
    }     
    CustomCommand_history.push(cmd);
    CustomCommand_history.slice(-40);
    CustomCommand_history_index = CustomCommand_history.length;
    marlin_addCommand(cmd);     
}

function CustomCommand_OnKeyUp(event) {
    if (event.keyCode == 13) {
        SendCustomCommand();
    }
    if (event.keyCode == 38 || event.keyCode == 40) {
        if (event.keyCode == 38 && CustomCommand_history.length > 0 && CustomCommand_history_index > 0) {
            CustomCommand_history_index--;
        } else if (event.keyCode == 40 && CustomCommand_history_index < CustomCommand_history.length - 1) {
            CustomCommand_history_index++;
        }

        if (CustomCommand_history_index >= 0 && CustomCommand_history_index < CustomCommand_history.length) {
            document.getElementById("custom_cmd_txt").value = CustomCommand_history[CustomCommand_history_index];
        }
        return false;
    }
    return true;
}

function marlin_setLightIRGB(I,R,G,B){
    // M150 [B<blue>] [P<Brightness >] [R<Red >] [U<Green >]-
    var cmd= "M150";
    if(typeof I != 'undefined' && I != null){
        cmd+=" P"+I;
    }
    if(typeof R != 'undefined' && R != null){
       cmd+=" R"+R;
    }
    if(typeof G != 'undefined' && G != null){
        cmd+=" U"+G;
    }
    if(typeof B != 'undefined' && B != null){
        cmd+=" B"+B;
    }
    marlin_addCommand(cmd);
}

var print_filename="";
function marlin_printFile_resp(response){
    on_httpStatusResponce(response);
    var cmd="M117 print:"+print_filename;
    cmd+="\nM300 P500 S400";//beep
    cmd+="\nM23 "+print_filename;
    cmd+="\nM24"
    marlin_addCommand(cmd);
}

function marlin_printFile(filename){
    var cmd="/setStateSDcontrol?mode=2";
    print_filename=filename;
    SendGetHttp(cmd,marlin_printFile_resp);
}