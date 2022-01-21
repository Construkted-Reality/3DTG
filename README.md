The **3DTG** tool will convert textured 3d mesh (and soon point cloud) formats into the 3D tiles format used for streaming.  

The tool is based on the open 3D Tile standard.  
https://github.com/CesiumGS/3d-tiles  
[3D Tiles Overview](https://github.com/CesiumGS/3d-tiles/blob/main/3d-tiles-overview.pdf) 


### Improvements
This is a list (in no particular order) of improvements that can be made:

- Caching mechanism (C++ equivalent to [Caffeine caching](https://github.com/ben-manes/caffeine) in Java)
- alternate decimation/simplification algorithms
- alternate texture baking/transfer algorithms in the creation of LODs
- Multiple input file formats (FBX, DAE/ZAE, GLTF/GLB)
- KTX 2.0 Basis texture encoding for better texture compression (better then jpg)
- Support point clouds (las/laz, e57 and other formats)
- add your own
