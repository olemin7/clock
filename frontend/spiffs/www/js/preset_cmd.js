var is_edit_mode=false;
var cmd_list=null;
var cmd_list_copy=null;

function preset_cmd_get() {
    var url = "/preset_cmd";
    SendGetHttp(url, process_preset_cmd_answer,function(){console.log("fail process_preset_cmd_answer")});
}

function preset_cmd_on_load(){
    preset_cmd_get();
}

function preset_cmd_edit_begin(){
    cmd_list_copy=[...cmd_list];
    $('#id_edit').prop('disabled', true);

    for(index=0;index<cmd_list.length;index++){
        $('#id_delete_'+index).show()
    };
    $('#id_add_cmd').show();
    $('#id_save').show();
    $('#id_cancel').show();
    is_edit_mode=true;
}

function preset_cmd_edit_exit(){
    $('#id_edit').prop('disabled', false);

    $('#id_save').hide();
    $('#id_cancel').hide();
    is_edit_mode=false;
    create_preset_cmd(cmd_list);
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
    $('#id_cmd_'+index).remove();
    cmd_list_copy.splice(index,1);
    console.log(cmd_list_copy);
    preset_cmd_in_edit_update();
}

var edit_index;
function preset_cmd_select(index){
    if(is_edit_mode){
        edit_index=Number(index);
        console.log(edit_index);
        if(-1!==edit_index){
            $('#id_edit_item_dialog_name').val(cmd_list_copy[edit_index].name);
            $('#id_edit_item_dialog_head').html("edit");
            $("#id_edit_item_handler").val(cmd_list_copy[edit_index].handler);
            $('#id_edit_item_val').val(cmd_list_copy[edit_index].val);
        }
        $('#id_edit_item_dialog').modal('show');
    }else{
//const found = array1.find(element => element > 10);
    }
}

function preset_cmd_save(){
    cmd_list=cmd_list_copy;
    preset_cmd_edit_exit();
}

function preset_cmd_cancel(){
    preset_cmd_edit_exit();
}

function preset_cmd_dialog_save(){
    const change = {name:$('#id_edit_item_dialog_name').val(),handler:$('#id_edit_item_handler').val(),val:$('#id_edit_item_val').val()}
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
        preset_cmd_in_edit_update();
    }else{
        cmd_list_copy[edit_index]=change;
        $('#id_cmd_name_'+edit_index).html(change.name);
    }
    $('#id_edit_item_dialog').modal('hide');
}

function preset_cmd_set_edit(edit){
    $('#id_edit').prop('disabled', !!edit);

    cmd_list.forEach(function(item){
        if(edit){
            $('#id_delete_'+item.name).show()
        }else{
            $('#id_delete_'+item.name).hide()
        }
    });
    if(edit){
        $('#id_add_cmd').show();
        $('#id_save').show();
        $('#id_cancel').show();
       
    } else {
        $('#id_add_cmd').hide();
        $('#id_save').hide();
        $('#id_cancel').hide();
    }
}

function create_button(item,index){
    console.log(item);
    var content  ='<div class="d-flex bd-highlight"';
        content +='id="id_cmd_'+index+'">'; 
        content +='<button type="button" class="btn btn-outline-primary flex-fill text-start"';
        content +='id="id_cmd_name_'+index+'"'; 
        content +='onclick="preset_cmd_select(\'' + index  + '\')"'
        content +='>'+ item.name+"</button>";
    
    content +='<button type="button" class="btn-close" id="id_delete_'+index+'" style="display: none;"'
    content +='onclick="preset_cmd_delete(\'' + index  + '\')">';
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
    content +='<button type="button" class="btn btn-outline-primary flex-fill text-start"';
    content += "onclick='preset_cmd_select(-1)' style='display: none;' id='id_add_cmd'>";
    content += "Add cmd</button>";

    $('#preset_cmd').html( content);
} 


function process_preset_cmd_answer(response_text) {
    var result = true;
    try { 
        var response = JSON.parse(response_text);
        console.log(response);
        cmd_list=response.items;
    } catch (e) {
        console.error("Parsing error:", e);
        result = false;
    }
    create_preset_cmd(cmd_list);
    return result;
}


