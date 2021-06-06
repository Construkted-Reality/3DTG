const {getAxisAlignedBoxFromBox, getLODFromNodeName, getOrientedBoundingBoxFromAxisAlignedBoundingBox} = require("./util");

function findChildren(rootNode, node, bBoxData, origModelName) {
    const level = node.level;
    const rootGeometricError = rootNode.geometricError;
    const geometricError = (rootGeometricError / rootNode.level) * (level - 1);
    const boundingBox = getAxisAlignedBoxFromBox(node.boundingVolume.box);

    const nodeNames = Object.keys(bBoxData);

    nodeNames.forEach(nodeName => {
        let lod = getLODFromNodeName(nodeName, origModelName);

        if(lod === level - 1) {
            // already have parent
            if(bBoxData[nodeName].parent)
                return;

            const nodeBoundingBox = getAxisAlignedBoxFromBox(bBoxData[nodeName]);

            /*
            if(nodeBoundingBox.minimum.x < boundingBox.minimum.x )
                return;

            if(nodeBoundingBox.minimum.y < boundingBox.minimum.y )
                return;

            if(nodeBoundingBox.minimum.z < boundingBox.minimum.z )
                return;

            if(nodeBoundingBox.maximum.x > boundingBox.maximum.x )
                return;

            if(nodeBoundingBox.maximum.y > boundingBox.maximum.y )
                return;

            if(nodeBoundingBox.maximum.z > boundingBox.maximum.z )
                return;
            */

            node.children.push({
                boundingVolume: {
                    box:  bBoxData[nodeName],
                    orientedBoundingBox: getOrientedBoundingBoxFromAxisAlignedBoundingBox(nodeBoundingBox)
                },
                geometricError: geometricError,
                level: level - 1,
                children: [],
                nodeName: nodeName
            });

            // set parent
            bBoxData[nodeName].parent = node;
        }
    });

    if(level === 1)
        return;

    for(let i = 0; i < node.children.length; i++) {
        findChildren(rootNode, node.children[i], bBoxData, origModelName);
    }
}

module.exports = findChildren;