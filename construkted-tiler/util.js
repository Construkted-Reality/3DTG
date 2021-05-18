const {AxisAlignedBoundingBox, Cartesian3} = require("cesium");

/**
 * @example
 *
 * const box = getAxisAlignedBoxFromBox([-16.023954391479492, -7.7196760177612305, -11.579096794128418, -0.6570236682891846,  3.080345869064331,  9.82688045501709])
 *
 * @param {array} box
 * @returns {module:cesium.AxisAlignedBoundingBox}
 */
function getAxisAlignedBoxFromBox(box) {
    const minimum = new Cartesian3(
        box[0],
        box[1],
        box[2],
    );

    const maximum = new Cartesian3(
        box[3],
        box[4],
        box[5],
    );

    return new AxisAlignedBoundingBox(minimum, maximum);
}

function getLODFromNodeName(nodeName, origModelName) {
    const start = (origModelName + "_LOD_").length;
    const end = start + 2;

    let lod = nodeName.substring(start, end);

    lod = parseInt(lod);

    if(isNaN(lod)) {
        console.warn("invalid lod detected.");
        return null;
    }

    return lod;
}

function getOrientedBoundingBoxFromAxisAlignedBoundingBox(axisAlignedBoundingBox) {
    const minimum = axisAlignedBoundingBox.minimum;
    const maximum = axisAlignedBoundingBox.maximum;

    const center = Cartesian3.add(minimum, maximum, new Cartesian3());

    Cartesian3.multiplyByScalar(center, 0.5, center);

    const scaleX = maximum.x - minimum.x;
    const scaleY = maximum.y - minimum.y;
    const scaleZ = maximum.z - minimum.z;

    // center and half axes of OrientedBoundingBox

    return [
        center.x,
        center.y,
        center.z,
        scaleX,
        0,
        0,
        0,
        scaleY,
        0,
        0,
        0,
        scaleZ
    ]
}

module.exports = {
    getOrientedBoundingBoxFromAxisAlignedBoundingBox: getOrientedBoundingBoxFromAxisAlignedBoundingBox,
    getLODFromNodeName: getLODFromNodeName,
    getAxisAlignedBoxFromBox: getAxisAlignedBoxFromBox
};