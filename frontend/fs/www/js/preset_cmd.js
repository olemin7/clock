function preset_cmd_on_load(){
    preset_cmd_get();
    preset_init();
}

//-------------- preset
function preset_button_caption(item){
    return '<strong>'+ item.name+ "</strong> ("+item.handler+":"+item.val  +")";
}

function preset_dialog_get(){
    return {name:$('#id_edit_item_dialog_name').val(),handler:$('#id_edit_item_handler').val(),val:$('#id_edit_item_val').val()}
}

function preset_onSave(list){
    console.log('not implemented');
}

function preset_isDublicate(lhs,rhs){
    return lhs.name===rhs.name;
}

function preset_send(item){
    console.log(item); 
    url="/command?handler="+item.handler;
    url+="&val="+item.val;
    SendGetHttp(url,undefined,on_ResponceErrorLog);
}

function present_edit_onOpen(index,item){
    if(-1!==edit_index){
        $('#id_edit_item_dialog_head').html("edit");
        $('#id_edit_item_dialog_name').val(item.name);
        $("#id_edit_item_handler").val(item.handler);
        $('#id_edit_item_val').val(item.val);
    }else{
        $('#id_edit_item_dialog_head').html("Add");
    }
}
//<------


function preset_cmd_get() {
    var url = "/config/preset_cmd.json";
    SendGetHttp(url, process_preset_cmd_answer,on_ResponceErrorLog);
}

// var edit_index;
// function preset_cmd_edit(index){
//     edit_index=Number(index);
//     console.log(edit_index);
//     if(-1!==edit_index){
//         $('#id_edit_item_dialog_name').val(cmd_list_copy[edit_index].name);
//         $('#id_edit_item_dialog_head').html("edit");
//         $("#id_edit_item_handler").val(cmd_list_copy[edit_index].handler);
//         $('#id_edit_item_val').val(cmd_list_copy[edit_index].val);
//     }else{
//         $('#id_edit_item_dialog_head').html("Add");
//     }
//     $('#id_edit_item_dialog').modal('show');
// }


function process_preset_cmd_answer(response_text) {
    try { 
        var response = JSON.parse(response_text);
        console.log(response);
        preset_begin(response.items);
    } catch (e) {
        console.error("Parsing error:", e);
    }
}


