var cmd_list=null;
var cmd_list_copy=null;

function preset_cmd_get() {
    var url = "/config/preset_cmd.json";
    SendGetHttp(url, process_preset_cmd_answer,on_ResponceErrorLog);
}

function preset_cmd_on_load(){
    preset_cmd_get();
    present_cmd_changed(false);
}

function present_cmd_changed(val){
    $('#id_save').prop('disabled', !val);
    $('#id_cancel').prop('disabled', !val);
}

function preset_cmd_edit_exit(){
    create_preset_cmd(cmd_list);
    present_cmd_changed(false);
}

function preset_cmd_in_edit_update(){
    create_preset_cmd(cmd_list_copy);
    for(index=0;index<cmd_list.length;index++){
        $('#id_delete_'+index).show();
    };
    $('#id_add_cmd').show();
}

function preset_cmd_delete(index){
    console.log("delete "+index);
    cmd_list_copy.splice(index,1);
    console.log(cmd_list_copy);
    preset_cmd_in_edit_update();
    present_cmd_changed(true);
}

var edit_index;
function preset_cmd_edit(index){
    edit_index=Number(index);
    console.log(edit_index);
    if(-1!==edit_index){
        $('#id_edit_item_dialog_name').val(cmd_list_copy[edit_index].name);
        $('#id_edit_item_dialog_head').html("edit");
        $("#id_edit_item_handler").val(cmd_list_copy[edit_index].handler);
        $('#id_edit_item_val').val(cmd_list_copy[edit_index].val);
    }else{
        $('#id_edit_item_dialog_head').html("Add");
    }
    $('#id_edit_item_dialog').modal('show');
}

function preset_cmd_save(){
    cmd_list=[...cmd_list_copy];
    //todo save to file
    preset_cmd_edit_exit();
}

function preset_cmd_cancel(){
    cmd_list_copy=[...cmd_list];
    preset_cmd_edit_exit();
}

function preset_cmd_dialog_get(){
    return {name:$('#id_edit_item_dialog_name').val(),handler:$('#id_edit_item_handler').val(),val:$('#id_edit_item_val').val()}
}

function preset_cmd_dialog_save(){
    const change = preset_cmd_dialog_get();
    if(0===change.name.length){
        alert("empty name");
        return;
    }
    const dublicate=cmd_list_copy.findIndex(function(element, index){
        if(index===edit_index){
            return false;
        }
        return element.name===change.name;
    })
    if(-1!==dublicate){
        alert("dublicate name with index "+dublicate);
        return;
    }

    if(-1===edit_index){
        cmd_list_copy.push(change);
    }else{
        cmd_list_copy[edit_index]=change;
    }
    preset_cmd_in_edit_update();
    $('#id_edit_item_dialog').modal('hide');
    present_cmd_changed(true);
}

function preset_cmd_send(cmd){
    console.log(cmd); 
        url="/command?handler="+cmd.handler;
        url+="&val="+cmd.val;
        SendGetHttp(url,undefined,on_ResponceErrorLog);
}

function create_button(item, index){
    console.log(item);
    var content  ='<div class="d-flex bd-highlight">';
        content +='<button type="button" class="btn btn-outline-primary flex-fill text-start"';
        content +='onclick="preset_cmd_send(cmd_list[' + index  + '])"'
        content +='><strong>'+ item.name+ "</strong> ("+item.handler+":"+item.val  +")</button>";
    content +='<button type="button" class="btn btn-outline-danger"'
    content +='onclick="preset_cmd_edit(\'' + index  + '\')">';
    content+=get_icon_svg("edit", "1.3em", "1.2em");
    content +='</button>';

    content +='<button type="button" class="btn btn-outline-danger"'
    content +='onclick="preset_cmd_delete(\'' + index  + '\')">';
    content+=get_icon_svg("remove", "1.3em", "1.2em");

    content +='</button>';
    content +="</div>"
    return content;
}

function create_preset_cmd(items){
    console.log(items);  
    var content = "";
    items.forEach(function(item,index){
        content += create_button(item,index);
    });
    $('#preset_cmd').html( content);
} 

function process_preset_cmd_answer(response_text) {
    var result = true;
    try { 
        var response = JSON.parse(response_text);
        console.log(response);
        cmd_list=response.items;
        cmd_list_copy=[...cmd_list];
    } catch (e) {
        console.error("Parsing error:", e);
        result = false;
    }
    create_preset_cmd(cmd_list);
    return result;
}


