## Introduction
The **3DTG** tool will convert textured 3d mesh (and soon point cloud) formats into the 3D tiles format used for streaming.  

The tool is based on the open [3D Tile standard](https://github.com/CesiumGS/3d-tiles). Also see [3D Tiles Overview](https://github.com/CesiumGS/3d-tiles/blob/main/3d-tiles-overview.pdf) 

## CLI Options

| Command         | Reuired                  | Initial value | Description                                                         |
|-----------------|--------------------------|---------------|---------------------------------------------------------------------|
| -h, --help      |                          |               | List all of the available commands for the current program version. |
| -i, --input     | <center>&check;</center> |               | Input model path                                                    |
| -o, --output    |                          | ./exported    | Output directory                                                    |
| -l, --limit     |                          | 2048          | Polygons limit (only for `Voxel, Regular` split alorithms)          |
| -g, --grid      |                          | 64            | Grid resolution (only for `Voxel` algorithm)                        |
| --iso           |                          |               | Currently unused                                                    |
| -a, --algorithm |                          | voxel         | Split algorithm to use                                              |
| --algorithms    |                          |               | Available algorithms list                                           |
| -f, --format    |                          | b3dm          | Model format to export                                              |
| --formats       |                          |               | Available formats list                                              |

### -h, --help
Prints an application help message into the CLI.

### -i, --input (`required`)
Input model path

***Example***
 > 3dtg -i ./someFolder/myModel.obj

***This option can be used in a positional way as a first argument***
 > 3dtg ./someFolder/myModel.obj

### -o, --output
Output directory path

***Example***
 > 3dtg ./someFolder/myModel.obj -o ./outdir

***This option can be used in a positional way as a second argument***
 > 3dtg ./someFolder/myModel.obj ./outdir

### -l, --limit
Polygons limit until model will be split

***Example***
 > 3dtg ./someFolder/myModel.obj ./outdir -l 4096

### -g, --grid
Grid size that is used to decimate voxelized meshes.

[Grid example](https://www.researchgate.net/profile/Hong-Liu-22/publication/230731211/figure/fig2/AS:300343045967883@1448618770846/Construct-voxel-grid-on-3D-point-cloud.png)

Should be set as a single value.
The higher is value the higher is the output resolution.

Default value is 64 which means [x, y, z] => [64, 64, 64] 

***Example***
 > 3dtg ./someFolder/myModel.obj ./outdir -g 32

### -a, --algorithm
Algorithm to use to split mesh

Default value is `voxel`

***Example***
 > 3dtg ./someFolder/myModel.obj ./outdir -a regular

### -f, --format
Model format to use to save models

Default value is `b3dm`

***Example***
 > 3dtg ./someFolder/myModel.obj ./outdir -f glb


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

## Dependencies

 * [GLTF](https://github.com/KhronosGroup/COLLADA2GLTF/tree/master/GLTF) : GLTF structures for encoding/decoding both a GLTF and a B3DM.
 * [draco](https://github.com/google/draco) : Draco compression lib for GLTF compression.
 * [rapidjson](https://github.com/Tencent/rapidjson) : JSON generation, used internally in GLTF lib to generate GLTF models.
 * [json](https://github.com/nlohmann/json) : JSON generation, used primarily.
 * [stb/images](https://github.com/nothings/stb) : Image module for encoding/decoding various image data formats.
 * [cxxopts](https://github.com/jarro2783/cxxopts) : CLI options parser.
 * [glm](https://github.com/g-truc/glm) : Math library for graphics.