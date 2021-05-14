const fs = require('fs');

const readGlbWriteB3dm = require("./lib/readGlbWriteB3dm");

const path = "lod_test.01";
const bboxDataJsonPath = path + "/export/bbox_data.json";
let bBoxRawData = fs.readFileSync(bboxDataJsonPath);

let bBoxData = JSON.parse(bBoxRawData);
console.log(bBoxData);

readGlbWriteB3dm("lod_test.01/export/RC-D01-02_LOD_00_0000.glb", "test.b3dm");

const asset = {
    version: 1.0,
    extra: {}
};