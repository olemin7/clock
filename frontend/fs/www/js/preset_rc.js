function preset_rc_on_load(){
    preset_init();
    preset_rc_get();
}
//-------------- preset
function preset_button_caption(item){
    return '<strong>'+ item.code+ "</strong> ("+item.cmd+")";
}

function preset_dialog_get(){
    return {code:Number($('#id_edit_item_dialog_rc').text()),cmd:$('#id_edit_item_cmd').val()}
}

function preset_onSave(list){
    console.log('not implemented');
}

function preset_isDublicate(lhs,rhs){
    console.log('not implemented');
}

function preset_send(item){
    console.log(item); 
    url="/command?handler="+item.handler;
    url+="&val="+item.val;
    SendGetHttp(url,undefined,on_ResponceErrorLog);
}

function present_edit_onOpen(index,item){
    if(-1!==edit_index){
        $('#id_edit_item_dialog_rc').text(item.code);
        $('#id_edit_item_cmd').val(item.cmd);
        $('#id_edit_item_dialog_head').html("edit");
    }else{
        $('#id_edit_item_dialog_head').html("Add");
    }
}
//<------

function preset_rc_get() {
    var url = "/config/preset_rc.json";
    SendGetHttp(url, process_preset_rc_answer,on_ResponceErrorLog);
    url = "/config/preset_cmd.json";
    SendGetHttp(url, process_preset_cmd_answer,on_ResponceErrorLog);
}

function process_preset_rc_answer(response_text) {
    try { 
        var response = JSON.parse(response_text);
        console.log(response);
        preset_begin(response.items);
    } catch (e) {
        console.error("Parsing error:", e);
    }
}

function process_preset_cmd_answer(response_text) {
    try { 
        var response = JSON.parse(response_text);
        console.log(response);
        response.items.forEach(function(cmd){
          $('#id_edit_item_cmd').append(`<option value="${cmd.name}"> ${cmd.name}</option>`);  
        })
    } catch (e) {
        console.error("Parsing error:", e);
    }
}





