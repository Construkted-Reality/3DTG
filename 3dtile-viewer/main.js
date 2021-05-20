const {app, BrowserWindow, ipcMain} = require('electron');
const path = require('path');
const fs = require('fs');
const {startServer, stopServer} = require('./server');
const {menu} = require("./menu");
const {dialog} = require('electron');
const isWindows = process.platform === "win32";

let mainWindow;

const openDevTool = false;

function createWindow() {
    mainWindow = new BrowserWindow({
        width: 800,
        height: 600,
        webPreferences: {
            enableRemoteModule: true,
            nodeIntegration: true,
            contextIsolation: false,
            preload: path.join(__dirname, 'preload.js')
        },
        frame: false //Remove frame to hide default menu
    });

    mainWindow.loadFile('./web-page/index.html');

    if(openDevTool)
        mainWindow.webContents.openDevTools();
}

app.whenReady().then(() => {
    createWindow();

    app.on('activate', () => {
        if (BrowserWindow.getAllWindows().length === 0) {
            createWindow()
        }
    });
});

app.on('window-all-closed', () => {
    if (process.platform !== 'darwin') {
        app.quit()
    }
});

ipcMain.on(`display-app-menu`, function (e, args) {
    if (isWindows && mainWindow) {
        menu.popup({
            window: mainWindow,
            x: args.x,
            y: args.y
        });
    }
});

ipcMain.on(`select-3d-tile-folder`, function (e, args) {
    if (isWindows && mainWindow) {
        const options = {
            title: 'Select a folder contains a tileset.json',
            properties: ['openDirectory'],
        };

        const path = dialog.showOpenDialogSync(mainWindow, options);

        if(!path)
            return;

        if(!fs.existsSync(path +'/tileset.json')){
            const messageBoxOptions = {
                type: "error",
                title: "Error",
                message: path + " does not contain tileset.json!"
            };

            dialog.showMessageBoxSync(messageBoxOptions);

            return;
        }

        const port = 3000;

        stopServer();
        startServer(port, path[0]);

        mainWindow.webContents.executeJavaScript("window.theApp.addTileset()");
    }
});

