var preset_list=null;
var preset_list_copy=null;


function preset_init(){
    present_changed(false);
}

function preset_begin(list) {
    preset_list = list;
    preset_list_copy=[...list]
    create_preset(preset_list);
    present_changed(false);
}

function present_changed(val){
    $('#id_save').prop('disabled', !val);
    $('#id_cancel').prop('disabled', !val);
}

// function preset_button_caption(item){
//  return button tetx
// }

function create_button(item, index){
    console.log(item);
    var content  ='<div class="d-flex bd-highlight">';
        content +='<button type="button" class="btn btn-outline-primary flex-fill text-start"';
        content +='onclick="preset_send(preset_list[' + index  + '])">'
        content +=preset_button_caption(item);
        content +='</button>';
    content +='<button type="button" class="btn btn-outline-danger"'
    content +='onclick="preset_edit(\'' + index  + '\')">';
    content+=get_icon_svg("edit", "1.3em", "1.2em");
    content +='</button>';

    content +='<button type="button" class="btn btn-outline-danger"'
    content +='onclick="preset_delete(\'' + index  + '\')">';
    content+=get_icon_svg("remove", "1.3em", "1.2em");

    content +='</button>';
    content +="</div>"
    return content;
}

function create_preset(items){
    console.log(items);  
    var content = "";
    items.forEach(function(item,index){
        content += create_button(item,index);
    });
    $('#preset').html( content);
} 

function preset_edit_exit(){
    create_preset(preset_list);
    present_changed(false);
}

function preset_in_edit_update(){
    create_preset(preset_list);
}

function preset_delete(index){
    console.log("delete "+index);
    preset_list.splice(index,1);
    preset_in_edit_update();
    present_changed(true);
}

var edit_index;
function preset_edit(index){
    edit_index=Number(index);
    console.log(edit_index);
    present_edit_onOpen(index,(-1===index)?undefined:preset_list[index]);
    $('#id_edit_item_dialog').modal('show');
}

function preset_save(){
    preset_list_copy=[...preset_list];
    preset_onSave(preset_list);
    preset_edit_exit();
}

function preset_cancel(){
    preset_list=[...preset_list_copy];
    preset_edit_exit();
}


function preset_dialog_save(){
    const change = preset_dialog_get();
    const dublicate=preset_list.findIndex(function(element, index){
        if(index===edit_index){
            return false;
        }
        return preset_isDublicate(element,change);
    })
    if(-1!==dublicate){
        alert("dublicate name with index "+dublicate);
        return;
    }

    if(-1===edit_index){
        preset_list.push(change);
    }else{
        preset_list[edit_index]=change;
    }
    preset_in_edit_update();
    $('#id_edit_item_dialog').modal('hide');
    present_changed(true);
}






