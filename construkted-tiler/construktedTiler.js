const fs = require('fs');

const {getAxisAlignedBoxFromBox, getLODFromNodeName, getOrientedBoundingBoxFromAxisAlignedBoundingBox} = require("./util");
const findChildren = require("./findChildren");
const readGlbWriteB3dm = require("./lib/readGlbWriteB3dm");

const importPath = "test/lod_test.01/export";
const bboxDataJsonPath = importPath + "/bbox_data.json";

if(!fs.existsSync(bboxDataJsonPath)) {
    console.error(`bboxDataJsonPath ${bboxDataJsonPath} does not exist!`);
    return;
}

const bBoxRawData = fs.readFileSync(bboxDataJsonPath);
const bBoxData = JSON.parse(bBoxRawData);

const exportPath = "test/lod_test.01_tileset/";

if(!fs.existsSync(exportPath)) {
    console.error(`exportPath ${exportPath} does not exist!`);
    return;
}

const origModelName = "RC-D01-02";

// find max lod
const nodeNames = Object.keys(bBoxData);

let maxLOD = 0;

nodeNames.forEach(nodeName => {
    let lod = getLODFromNodeName(nodeName, origModelName);

    lod = parseInt(lod);

    if(isNaN(lod)) {
        console.error("invalid lod detected.");
        return;
    }

    if(lod > maxLOD)
        maxLOD = lod;
});

// find root node name
let rootNodeName;

nodeNames.forEach(nodeName => {
    let lod = getLODFromNodeName(nodeName, origModelName);

    if(lod === maxLOD) {
        rootNodeName = nodeName;
    }
});

if(!rootNodeName){
    console.error("failed to get root node name.");
    return;
}

// make root node
const root = {
    boundingVolume: {
        box:  bBoxData[rootNodeName],
        orientedBoundingBox: getOrientedBoundingBoxFromAxisAlignedBoundingBox( getAxisAlignedBoxFromBox(bBoxData[rootNodeName]))
    },
    geometricError: 1,
    level: maxLOD,
    children: [],
    nodeName: rootNodeName
};

// recursively find children
findChildren(root, root, bBoxData, origModelName);

// recursively remove unnecessary properties and generate glb files
removeUnnecessaryPropertiesAndGenerateContents(root);

// write tileset json
const tilesetJson = {
    asset: {
        version: "1.0",
        extra: {}
    },
    root: root
};

const data = JSON.stringify(tilesetJson, null, 2);

fs.writeFileSync(exportPath + "/tileset.json", data);

function removeUnnecessaryPropertiesAndGenerateContents(node) {
    const dir = exportPath + "/" + node.level;

    if (!fs.existsSync(dir)){
        fs.mkdirSync(dir);
    }

    const glbPath = importPath + "/" + node.nodeName + ".glb";

    const contentUri = node.level + "/" + node.nodeName + ".b3dm";
    const b3dmPath = exportPath + "/" + contentUri;

    readGlbWriteB3dm(glbPath , b3dmPath, true);

    node.boundingVolume.box = node.boundingVolume.orientedBoundingBox;

    delete node.boundingVolume.orientedBoundingBox;
    delete node.level;
    delete node.nodeName;

    node.content = {
        uri : contentUri
    };

    for(let i = 0; i < node.children.length; i++) {
        removeUnnecessaryPropertiesAndGenerateContents(node.children[i]);
    }
}




