const {remote} = require("electron");
/**
 *
 * note that following does not work
 * const { dialog } = require("electron").remote.dialog;
 */

const dialog = remote.dialog;

const ipcRenderer = require('electron').ipcRenderer;

window.addEventListener("DOMContentLoaded", () => {
    const menuButton = document.getElementById("menu-btn");
    const minimizeButton = document.getElementById("minimize-btn");
    const maxUnmaxButton = document.getElementById("max-unmax-btn");
    const closeButton = document.getElementById("close-btn");

    menuButton.addEventListener("click", e => {
        window.openMenu(e.x, e.y);
    });

    minimizeButton.addEventListener("click", e => {
        window.minimizeWindow();
    });

    maxUnmaxButton.addEventListener("click", e => {
        const icon = maxUnmaxButton.querySelector("i.far");

        window.maxUnmaxWindow();

        if (window.isWindowMaximized()) {
            icon.classList.remove("fa-square");
            icon.classList.add("fa-clone");
        } else {
            icon.classList.add("fa-square");
            icon.classList.remove("fa-clone");
        }
    });

    closeButton.addEventListener("click", e => {
        window.closeWindow();
    });

    document.querySelector('#select-3d-tileset').addEventListener('click', function (event) {
        ipcRenderer.send('select-3d-tile-folder')
    });
});
