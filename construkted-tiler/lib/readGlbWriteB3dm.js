const Cesium = require('cesium');
var fsExtra = require('fs-extra');
var zlib = require('zlib');
var Promise = require('bluebird');
var glbToB3dm = require('./glbToB3dm');

var fileExists = require('./fileExists');
var isGzipped = require('./isGzipped');

var zlibGunzip = Promise.promisify(zlib.gunzip);

var defaultValue = Cesium.defaultValue;
var DeveloperError = Cesium.DeveloperError;

function readFile(file) {
    return fsExtra.readFile(file)
        .then(function(fileBuffer) {
            if (isGzipped(fileBuffer)) {
                return zlibGunzip(fileBuffer);
            }
            return fileBuffer;
        });
}

function checkFileOverwritable(file, force) {
    if (force) {
        return Promise.resolve();
    }
    return fileExists(file)
        .then(function (exists) {
            if (exists) {
                throw new DeveloperError('File ' + file + ' already exists. Specify -f or --force to overwrite existing files.');
            }
        });
}

function readGlbWriteB3dm(inputPath, outputPath, force) {
    outputPath = defaultValue(outputPath, inputPath.slice(0, inputPath.length - 3) + 'b3dm');
    return checkFileOverwritable(outputPath, force)
        .then(function() {
            return readFile(inputPath)
                .then(function(glb) {
                    // Set b3dm spec requirements
                    var featureTableJson = {
                        BATCH_LENGTH : 0
                    };
                    return fsExtra.outputFile(outputPath, glbToB3dm(glb, featureTableJson));
                });
        });
}

module.exports = readGlbWriteB3dm;