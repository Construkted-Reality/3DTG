/*eslint-env node*/
/* eslint-disable no-unused-vars */
/* eslint-disable global-require */

"use strict";

let server;

function startServer(port, dir) {
    const express = require("express");
    const compression = require("compression");
    const fs = require("fs");
    const url = require("url");
    const gzipHeader = Buffer.from("1F8B08", "hex");

    // eventually this mime type configuration will need to change
    // https://github.com/visionmedia/send/commit/d2cb54658ce65948b0ed6e5fb5de69d022bef941
    // *NOTE* Any changes you make here must be mirrored in web.config.
    const mime = express.static.mime;
    mime.define(
        {
            "application/json": ["czml", "json", "geojson", "topojson"],
            "application/wasm": ["wasm"],
            "image/crn": ["crn"],
            "image/ktx": ["ktx"],
            "model/gltf+json": ["gltf"],
            "model/gltf-binary": ["bgltf", "glb"],
            "application/octet-stream": [
                "b3dm",
                "pnts",
                "i3dm",
                "cmpt",
                "geom",
                "vctr",
            ],
            "text/plain": ["glsl"],
        },
        true
    );

    const app = express();
    app.use(compression());
    app.use(function (req, res, next) {
        res.header("Access-Control-Allow-Origin", "*");
        res.header(
            "Access-Control-Allow-Headers",
            "Origin, X-Requested-With, Content-Type, Accept"
        );
        next();
    });

    function checkGzipAndNext(req, res, next) {
        const reqUrl = url.parse(req.url, true);
        const filePath = reqUrl.pathname.substring(1);

        const readStream = fs.createReadStream(filePath, { start: 0, end: 2 });
        readStream.on("error", function (err) {
            next();
        });

        readStream.on("data", function (chunk) {
            if (chunk.equals(gzipHeader)) {
                res.header("Content-Encoding", "gzip");
            }
            next();
        });
    }

    const knownTilesetFormats = [
        /\.b3dm/,
        /\.pnts/,
        /\.i3dm/,
        /\.cmpt/,
        /\.glb/,
        /\.geom/,
        /\.vctr/,
        /tileset.*\.json$/,
    ];

    app.get(knownTilesetFormats, checkGzipAndNext);

    app.use(express.static(dir));

    server = app.listen(
        port,
        "localhost",
        function () {
            console.log(
                "Cesium development server running locally.  Connect to http://localhost:%d/",
                server.address().port
            );
        }
    );

    server.on("error", function (e) {
        if (e.code === "EADDRINUSE") {
            console.log(
                "Error: Port %d is already in use, select a different port.", port
            );
            console.log("Example: node server.cjs --port %d", port + 1);
        } else if (e.code === "EACCES") {
            console.log(
                "Error: This process does not have permission to listen on port %d.", port
            );
            if (port < 1024) {
                console.log("Try a port number higher than 1024.");
            }
        }

        console.log(e);
        process.exit(1);
    });

    server.on("close", function () {
        console.log("Cesium development server stopped.");
    });

    let isFirstSig = true;

    process.on("SIGINT", function () {
        if (isFirstSig) {
            console.log("Cesium development server shutting down.");
            server.close(function () {
                process.exit(0);
            });
            isFirstSig = false;
        } else {
            console.log("Cesium development server force kill.");
            process.exit(1);
        }
    });
}

function stopServer() {
    if(server)
        server.close();
}

module.exports = {
    startServer,
    stopServer
};