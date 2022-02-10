function selectTab(index) {
    const navs = document.getElementsByClassName("nav");
    if (navs.length > 0) {
        const menuItems = navs[0].getElementsByTagName("li");
        for (let c = 0; c < menuItems.length; c++) {
            const item = menuItems[c];
            item.getElementsByTagName("a")[0].classList.remove("active");
        }
        const link = menuItems[index].getElementsByTagName("a")[0];
        link.classList.add("active");

        const panels = document.getElementsByClassName("panel");
        for (let c = 0; c < panels.length; c++)
            panels[c].classList.add("hide");

        document.getElementById(link.getAttribute("href").substring(1)).classList.remove("hide");
    }
}
function logoutButton() {
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/logout", true);
    xhr.send();
    window.open("/logout", "_self");
}
function rebootButton() {
    document.getElementById("statusdetails").innerHTML =
        "Invoking Reboot ...";
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/reboot", true);
    xhr.send();
    window.open("/reboot", "_self");
}
function listFilesButton() {
    const spinnerElement = document.getElementById("spinner");
    spinnerElement.classList.remove("hide");
    xmlhttp = new XMLHttpRequest();
    xmlhttp.open("GET", "/listfiles", false);
    xmlhttp.send();
    document.getElementById("detailsheader").innerHTML = "<h3>Files<h3>";
    document.getElementById("details").innerHTML = xmlhttp.responseText;
    spinnerElement.classList.add("hide");
}
function downloadDeleteButton(filename, action) {
    var urltocall = "/file?name=" + filename + "&action=" + action;
    xmlhttp = new XMLHttpRequest();
    if (action == "delete") {
        xmlhttp.open("GET", urltocall, false);
        xmlhttp.send();
        document.getElementById("status").innerHTML = xmlhttp.responseText;
        xmlhttp.open("GET", "/listfiles", false);
        xmlhttp.send();
        document.getElementById("details").innerHTML = xmlhttp.responseText;
    }
    if (action == "download") {
        document.getElementById("status").innerHTML = "";
        window.open(urltocall, "_blank");
    }
}
function _(el) {
    return document.getElementById(el);
}
function uploadFile() {
    var file = _("file1").files[0];
    // alert(file.name+" | "+file.size+" | "+file.type);
    var formdata = new FormData();
    formdata.append("file1", file);
    var ajax = new XMLHttpRequest();
    ajax.upload.addEventListener("progress", progressHandler, false);
    ajax.addEventListener("load", completeHandler, false); // doesnt appear to ever get called even upon success
    ajax.addEventListener("error", errorHandler, false);
    ajax.addEventListener("abort", abortHandler, false);
    ajax.open("POST", "/");
    ajax.send(formdata);
}
function progressHandler(event) {
    //_("loaded_n_total").innerHTML = "Uploaded " + event.loaded + " bytes of " + event.total; // event.total doesnt show accurate total file size
    _("loaded_n_total").innerHTML = "Uploaded " + event.loaded + " bytes";
    var percent = (event.loaded / event.total) * 100;
    _("progressBar").value = Math.round(percent);
    _("status").innerHTML =
        Math.round(percent) + "% uploaded... please wait";
    if (percent >= 100) {
        _("status").innerHTML = "Please wait, writing file to filesystem";
    }
}
function completeHandler(event) {
    _("status").innerHTML = "Upload Complete";
    _("progressBar").value = 0;
    xmlhttp = new XMLHttpRequest();
    xmlhttp.open("GET", "/listfiles", false);
    xmlhttp.send();
    document.getElementById("status").innerHTML = "File Uploaded";
    document.getElementById("detailsheader").innerHTML = "<h3>Files<h3>";
    document.getElementById("details").innerHTML = xmlhttp.responseText;
    document.getElementById("uploadModal").style.display = "none";
}
function errorHandler(event) {
    _("status").innerHTML = "Upload Failed";
}
function abortHandler(event) {
    _("status").innerHTML = "inUpload Aborted";
}
function onload() {
    var modal = document.getElementById("uploadModal");
    var span = document.getElementsByClassName("close")[0];
    span.onclick = function () {
        modal.style.display = "none";
    }
    window.onclick = function (event) {
        if (event.target == modal)
            modal.style.display = "none";
    }
}
function showUploadModal() {
    var modal = document.getElementById("uploadModal");
    modal.style.display = "block";
}