function fsbrowser(id_holder_filelist,impl_name_,file_select,editbtn_){
    var impl_name=impl_name_;    
    var holder_filelist = document.getElementById(id_holder_filelist);
    var file_select_func=file_select;
    const editbtn=(typeof editbtn_ != 'undefined' && editbtn_==true)?true:false;

    var curfileListId=[];
    var selectedFolder="/";
    var selectedFile="";
    CreateNavBar(selectedFolder);
    

    function ResultSuccess(response){
        var jsonresponse = JSON.parse(response);
        console.log(jsonresponse);
        CreateNavBar(jsonresponse.path);
        CreateFileList(jsonresponse.files)
    }

    function ResultError(response){
        holder_filelist.innerHTML +="Error"+ response;
    }

    function CreateNavBarItem(name,path){
        var content='<button class="btn btn-light" onclick="'+impl_name+".openDir(\'"+path+"\')\" >/";
        content+= name;
        content+="</button>";
        return content;
    } 

    function CreateNavBar(path){    
        selectedFolder=path;        
        var content="";       
        var path_item = "/";         
        content+=CreateNavBarItem("home",path_item);
        path.split("/").forEach(function(element){ 
            if(element!=""){
                path_item+= element+"/";
                content+=CreateNavBarItem(element,path_item);
            }
        });
        id_fsbrowser_navbar.innerHTML = content;
    }

    function CreateFileList(files){
        var content ='<div class="row">';        
        content+='<div class="w-100">'
        if (typeof files != 'undefined' && files.length){
            files.forEach(function(file){
                const FineName=file[0];
                const isDir=file[1];
                const Size=file[2];
                curfileListId.push(file);
                content += '<div class="row"><div class="col m-1">'
                if(isDir){
                    content +='<button type="button" class="btn btn-outline-secondary btn-block text-left"';
                }else{
                    content +='<button type="button" class="btn btn-outline-primary btn-block text-left"';
                }
                content +=' onclick="'+impl_name+".select(\'"+FineName+"\')\" id=\'"+impl_name+FineName+"\'>";
                content+=FineName;
                if(!isDir){
                    content+=" ,sz:"+Size;
                }
                content+="</button></div>";
                content += '<div class="col-md-auto m-1">'
                if(!isDir){
                    content +='<a href="/filedownload?file='+selectedFolder+FineName+'" download="'+FineName+'"  class="btn btn-info m-1" >';
                    content+= get_icon_svg("download")
                    content+='</a>';
                    content+='<button class="btn btn-warning m-1" onclick="marlin_printFile(\''+selectedFolder+FineName+"\')\" >";
                    content+= get_icon_svg("print");
                    content+='</button>';
                }
                if(editbtn){                                        
                    content+='<button class="btn btn-danger m-1" onclick="'+impl_name+".delete(\'"+FineName+"\')\" >";
                    content+= get_icon_svg("trash");
                    content+='</button>';
                }
                content+='</div></div><div class="w-100">';
            });        
        }else{
            content += 'empty...';
        }
        content += "</div>";
        holder_filelist.innerHTML = content;
    }

    this.openDir = function(dir){        
        console.log("openDir="+dir);
        var url ="/filelist?path="+ encodeURI(dir);
        curfileListId=[];
        console.log(url);
        selectedFile="";
        SendGetHttp(url,ResultSuccess,ResultError);
    }

    this.refresh = function(){
        this.openDir(selectedFolder);
    }    

    this.select = function(item){
        console.log(item);
        if(item.endsWith("/")){//directore
            this.openDir(selectedFolder+item);
        }else{
            if(selectedFile!="")
                setActiveState(impl_name+selectedFile,false);
            if(selectedFile!=item){
                setActiveState(impl_name+item,true);
                selectedFile=item;
            }else{//double select= remove selection
                selectedFile="";
            }
        }
        if (typeof file_select_func != 'undefined'){
            file_select_func(this.getSelectedFile());
        }
    }

    this.mkdir = function(){
        var dir = prompt("mkdir in :"+ selectedFolder, "newdir");
        if (dir != null) {
            var url ="/filelist?mkdir="+ encodeURI(selectedFolder+dir);
            console.log(url);
            SendGetHttp(url,Uploadsuccess.bind(null,this));
        }
    }    

    this.delete = function(item){
        if (confirm('Delete: "'+item+'"?')){
            var url ="/filelist?delete="+ encodeURI(item);
            console.log(url);
            SendGetHttp(url,Uploadsuccess.bind(null,this),ResultError);
        }
    }

    this.getSelectedFile=function(){        
        if(selectedFile=="")
            return "";
        return selectedFolder+selectedFile;
    }


    //this.refresh();


function UploadProgressDisplay(oEvent){
    if (oEvent.lengthComputable) {
        var percentComplete = (oEvent.loaded / oEvent.total)*100;
        holder_filelist.innerHTML += " " + percentComplete.toFixed(0)+"%" ;
    }
}

    this.UploadFile=function (files){
        if (files.length){
            var formData = new FormData();
            var url = "/filelist";
            holder_filelist.innerHTML += "Uploading";
            formData.append('path', selectedFolder);
            for (var i = 0; i < files.length; i++) {
                var file = files[i];
                var arg = selectedFolder + file.name + "S";
                //append file size first to check updload is complete
                formData.append(arg, file.size);
                formData.append('myfile[]', file, selectedFolder + file.name);
            }
            //console.log(formData);
            SendFileHttp(url, formData, UploadProgressDisplay, Uploadsuccess.bind(null,this), Uploadfailed.bind(null,this))
        }else{
             console.log("no file");
    }
}

function Uploadsuccess(imp,response){
    holder_filelist.innerHTML += "done";
//    imp.refresh();
    var status={};
    status.fsChanged=true;
    on_statusUpdate(status);
}

function Uploadfailed(imp,errorcode, response){    
    holder_filelist.innerHTML += "failed :"+ errorcode + " :" + response;
    console.log("Error " + errorcode + " : " + response);    
}

}

function init_fsbrowser() {
    setStateSDcontrol();
}

function setStateSDcontrol(mode) {
    var cmd="/setStateSDcontrol";
    if (typeof mode != 'undefined'){
        cmd+="?mode="+mode;
    }
    SendGetHttp(cmd,on_httpStatusResponce);
}

function setActiveState(id,isActive) {
    if(isActive){
        document.getElementById(id).classList.add("active");
    }else{
        document.getElementById(id).classList.remove("active");
    }
}

function fsbrowser_UpdateStatus(status){
    if((typeof status.stateSDcontrol)!='undefined'){
        setActiveState('SDcontrol_esp',(status.stateSDcontrol==SDcontrolMode_esp));
        setActiveState('SDcontrol_marlin',(status.stateSDcontrol==SDcontrolMode_marlin));
    }
    if((typeof status.fsChanged)!='undefined'){
        fsbrowser_impl.refresh();
    }
}

