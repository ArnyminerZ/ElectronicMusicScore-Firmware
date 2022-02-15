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
 * @param {String?} p The path to make the request to.
 */
const G = (p) => {
    try {
        var xhr = new XMLHttpRequest();
        xhr.open("GET", p, false);
        xhr.send();
        return xhr.responseText;
    } catch (e) {
        console.error("Could not load", p, ". Error:", e);
        return "";
    }
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

/**
 * Displays the text [t] in the snackbar.
 * @param {String} t The text to display.
 * @param {Number} d The amount of time in ms the snackbar will be shown.
 */
const sb = (t, d = 3000) => {
    const s = _("sb");
    CA(s, "show"); // Shows the snackbar
    H(s, t); // Set the text
    setTimeout(() => CR(s, "show"), d); // Hides after d ms
}

/**
 * Converts [size] into a human readable string.
 * @param {Number} size The size to convert in bytes.
 * @return {String} [size] in a "human readable" format.
 */
function fileSize(size) {
    var i = Math.floor(Math.log(size) / Math.log(1024));
    return (size / Math.pow(1024, i)).toFixed(2) * 1 + ' ' + ['B', 'kB', 'MB', 'GB', 'TB'][i];
}

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

/**
 * Configures the parameter [key] to [value].
 * @param {String} key The config key to use
 * @param {String} value The value to pass to the configurer to set config [key].
 */
const config = (key, value) => {
    G(`/config?key=${key}&value=${value}`);
}

let filesItemTemplate;
function listFiles() {
    const spinnerElement = _("spinner");
    CR(spinnerElement, "hide");
    const c = _("filesTable");
    try {
        const fs = JSON.parse(G("/listfiles")).files;
        H(c); // Clear the table container
        for (let f of fs) {
            const n = f.name; // The name of the file
            const ep = n.lastIndexOf("."); // The position of the last point/extension
            HA(
                c,
                filesItemTemplate
                    .replaceAll("{TITLE}", n.substring(0, ep))
                    .replaceAll("{TYPE}", n.substring(ep + 1).toUpperCase())
                    .replaceAll("{SIZE}", fileSize(f.size))
                    .replaceAll("{FILENAME}", f.name)
            );
        }
    } catch (e) {
        console.error("Could not load the list of files. Error:", e);
    } finally {
        CA(spinnerElement, "hide");
    }
}
function fileAction(filename, action) {
    var uc = `/file?name=${filename}&action=${action}`;
    if (action == "delete") {
        CR(_("spinner"), "hide");
        try {
            // Call the backend for deleting the file
            G(uc)
            // If no errors ocurred, list files and show snackbar
            listFiles();
            sb("Deleted file");
        } catch (e) {
            // If an errors occurs, show snackbar
            sb("Could not delete file");
            console.error("Could not delete file. Error:", e);
        }
        CA(_("spinner"), "hide");
    }
    if (action == "download")
        NB(uc);
}
function uploadFile() {
    var file = _("uploadFi").files[0];
    var formdata = new FormData();
    formdata.append("uploadFi", file);
    var ajax = new XMLHttpRequest();
    ajax.upload.addEventListener("progress", (event) => {
        H(_("loaded_n_total"), `Uploaded ${event.loaded} bytes`);
        var percent = Math.round((event.loaded / event.total) * 100);

        _("uploadPB").value = percent;
        H(_("uploadSt"), `${percent}% uploaded... please wait`);
        if (percent >= 100)
            H(_("uploadSt"), "Please wait, writing file to filesystem");
    }, false);
    ajax.addEventListener("load", () => {
        // Hide modal
        _("uploadModal").style.display = "none";
        // Show Snackbar
        sb("File Uploaded!");
        // Empty status field
        H(_("uploadSt"));
        // Reset upload progress bar
        _("uploadPB").value = 0;
        // Clear selected file
        _("uploadFi").value = null;
        // List all the files again
        // TODO: Instead of loading all files again, add file dynamically
        listFiles();
    }, false); // doesnt appear to ever get called even upon success
    ajax.addEventListener("error", () => {
        sb("Upload Failed!");
    }, false);
    ajax.addEventListener("abort", () => {
        sb("Upload Aborted!")
    }, false);
    ajax.open("POST", "/");
    ajax.send(formdata);
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
    let c = 0;
    for (i of r.split(";")){
        if (i.length > 0) {
            // First element is session, second creation date
            const u = i.split(",");
            HA(e, `<tr id="ses${i}"><td>${u[0]}</td><td><button type="button" onclick="config('delSession',${c});_('ses${i}').remove();">Delete</button></td></tr>`);
        }
        c++;
    }

    // Load the default template for files
    filesItemTemplate = _("filesTable").innerHTML; // The template for files list

    // List the files in the SPIFFS
    listFiles();
}
function showUploadModal() {
    _("uploadModal").style.display = "block";
}