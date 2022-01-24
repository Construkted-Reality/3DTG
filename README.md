## Introduction
The **3DTG** tool will convert textured 3d mesh (and soon point cloud) formats into the 3D tiles format used for streaming.  

The tool is based on the open [3D Tile standard](https://github.com/CesiumGS/3d-tiles). Also see [3D Tiles Overview](https://github.com/CesiumGS/3d-tiles/blob/main/3d-tiles-overview.pdf) 

## Functionality
### Current Functionality 
Currently the tool only accepts textured OBJ files, and simplifies the geometry using a voxel decimation algorithm. 
The output is a 3d tiles.

Syntax:  
`3dtg.exe -i path\to\input\obj -o path\to\3d-tile\folder`

### Memory usage
A rough estimate of memory usage can be calculated by adding up the number of textures and the size of the uncompressed 3d mesh.
16k textures use about 1GB ram
8k textures use about 0.25 GB of ram (256MB)
4k textures use about 0.06 GB ram (64MB)

System ram usage = (Mesh_in_OBJ_format) + (#_of_textures * Texture_Resolution)

This will provide a rough ballpark on the amount of system ram required for processing your textured mesh.


### Future Improvements
This is a list (in no particular order) of improvements that can be made:

- Caching mechanism (C++ equivalent to [Caffeine caching](https://github.com/ben-manes/caffeine) in Java)
- alternate decimation/simplification algorithms
- alternate texture baking/transfer algorithms in the creation of LODs
- Optomizations for increased processing performance
- GPU acceleration for faster overall processing
- Multiple input file formats (FBX, DAE/ZAE, GLTF/GLB)
- KTX 2.0 Basis texture encoding for better texture compression (better then jpg)
- Support point clouds (las/laz, e57 and other formats)
- add your own!
