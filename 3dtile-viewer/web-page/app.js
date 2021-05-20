class App {
    constructor() {
        const viewer = new Cesium.Viewer("cesiumContainer", {});

        viewer.extend(Cesium.viewerCesium3DTilesInspectorMixin);

        viewer.extend(Cesium.viewerMeasureMixin, {
            units: new Cesium.MeasureUnits({
                distanceUnits: Cesium.DistanceUnits.METERS,
                areaUnits: Cesium.AreaUnits.SQUARE_METERS,
                volumeUnits: Cesium.VolumeUnits.CUBIC_METERS
            })
        });

        this._viewer = viewer;
    }

    addTileset() {
        const viewer = this._viewer;

        if(this._tileset) {
            viewer.scene.primitives.remove(this._tileset);
        }

        const tileset = new Cesium.Cesium3DTileset({
            url: 'http://localhost:3000/tileset.json'
        });

        viewer.scene.primitives.add(tileset);

        this._tileset = tileset;

        tileset.readyPromise.then(function () {
            viewer.zoomTo(tileset);
        }).otherwise((error) => {
            console.error(error);
        });
    }
}

export {App};