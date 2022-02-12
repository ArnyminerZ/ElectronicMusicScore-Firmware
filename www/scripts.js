const _ = (el) => document.getElementById(el);
/**
 * Selects a child of [p] based on [q].
 * @param {DOMElement} p The parent element to get the children from.
 * @param {String} q The query selector for the children.
 * @returns {DOMElement}
 */
const $ = (q, p = null) => (p == null ? document : p).querySelector(q);

/**
 * Selects all children of [p] based on [q].
 * @param {DOMElement} p The parent element to get the children from.
 * @param {String} q The query selector for the children.
 * @returns {DOMElement}
 */
const S = (q, p = null) => (p == null ? document : p).querySelectorAll(q);

/**
 * Sets the innerHTML parameter of [e] to [h];
 * @param {DOMElement} e The element to modify.
 * @param {String} h The HTML code to set.
 * @returns The operation result
 */
const H = (e, h = "") => e.innerHTML = h;

/**
 * Appends [h] to the innerHTML parameter of [e];
 * @param {DOMElement} e The element to modify.
 * @param {String} h The HTML code to append.
 * @returns The operation result
 */
const HA = (e, h = "") => e.innerHTML += h;

/**
 * Makes a GET request to [p].
 * @param {String} p The path to make the request to.
 */
const G = (p) => {
    var xhr = new XMLHttpRequest();
    xhr.open("GET", p, false);
    xhr.send();
    return xhr.responseText;
}

/**
 * Adds a class to the class list of [e].
 * @param {DOMElement} e The element to add the class to
 * @param {String} c The class to add to [e].
 */
const CA = (e, c) => e.classList.add(c);

/**
 * Removes a class from the class list of [e].
 * @param {DOMElement} e The element to remove the class from
 * @param {String} c The class to remove to [e].
 */
const CR = (e, c) => e.classList.remove(c);

/**
 * Opens the path [p] on self.
 * @param {String} p The path to load.
 * @returns 
 */
const N = (p) => window.open(p, "_self");

/**
 * Opens the path [p] on _blank.
 * @param {String} p The path to load.
 * @returns 
 */
const NB = (p) => window.open(p, "_blank");

function selectTab(i) {
    // Get all children elements of .nav of type li
    const mi = S("li", $(".nav"));
    for (let it of mi)
        CR($("a", it), "active"); // Remove class active from child a in item

    const a = $("a", mi[i]);

    // Remove class active from first child a of element i in menuItems
    CA(a, "active");

    for (let p of S(".panel"))
        CA(p, "hide");

    CR(_(a.getAttribute("href").substring(1)), "hide");
}
function logoutButton() {
    G("/logout");
    N("/logout");
}
function rebootButton() {
    H(_("statusdetails"), "Invoking Reboot ...");
    G("/reboot");
    N("/reboot");
}
function listFilesButton() {
    const spinnerElement = _("spinner");
    CR(spinnerElement, "hide");
    H(_("detailsheader"), "<h3>Files</h3>");
    H(_("details"), G("/listfiles"))
    CA(spinnerElement, "hide");
}
function downloadDeleteButton(filename, action) {
    var uc = `/file?name=${filename}&action=${action}`;
    if (action == "delete") {
        H(_("status"), G(uc));
        H(_("details"), G("/listfiles"));
    }
    if (action == "download") {
        H(_("status"));
        NB(uc);
    }
}
function uploadFile() {
    var file = _("file1").files[0];
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
    H(_("loaded_n_total"), `Uploaded ${event.loaded} bytes`);
    var percent = Math.round((event.loaded / event.total) * 100);
    _("progressBar").value = percent;
    H(_("status"), `${percent}% uploaded... please wait`);
    if (percent >= 100)
        H(_("status"), "Please wait, writing file to filesystem");
}
function completeHandler(event) {
    _("status").innerHTML = "Upload Complete";
    _("progressBar").value = 0;
    _("details").innerHTML = G("/listfiles");
    _("status").innerHTML = "File Uploaded";
    _("detailsheader").innerHTML = "<h3>Files<h3>";
    _("uploadModal").style.display = "none";
}
function errorHandler(event) {
    H(_("status"), "Upload Failed");
}
function abortHandler(event) {
    H(_("status"), "Upload Aborted");
}
function onload() {
    // Load the modal events
    const modal = _("uploadModal");
    $(".close").onclick = () => {
        modal.style.display = "none";
    };
    window.onclick = (event) => {
        if (event.target == modal)
            modal.style.display = "none";
    };
    // Load the sessions
    const e = _("sessionsBody");
    const s = _("authSessions").value;
    const r = s.substring(s.indexOf("^") + 1);
    H(e); // Clear the children
    for (i of r.split(";"))
        if (i.length > 0) {
            // First element is session, second creation date
            const u = i.split(",");
            HA(e, `<tr><td>${u[0]}</td><td><button onclick="config('delete_session=${i}')">Delete</button></td></tr>`);
        }
}
function showUploadModal() {
    _("uploadModal").style.display = "block";
}