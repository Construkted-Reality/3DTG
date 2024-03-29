## Introduction
The **3DTG** tool will process textured 3D meshes (and soon point clouds) and convert them into the [Cesium 3D Tiles standard](https://github.com/CesiumGS/3d-tiles) used for streaming large datasets on the web.
Also see [3D Tiles Overview](https://github.com/CesiumGS/3d-tiles/blob/main/3d-tiles-overview.pdf) 

## Build dependencies

`make`, `cmake` and `g++` commands should be available on your OS.
Also, `posix` threads should be available for the C++ compiler.

 1. `make` command can be provided through the [GNU Make](https://www.gnu.org/software/make/)
 2. `cmake` command can be provided through the [CMake](https://cmake.org/)

### Win

 1. `make` command can be provided through the [GNU Make](https://www.gnu.org/software/make/)
 2. `cmake` command can be provided through the [CMake](https://cmake.org/)
 3. `g++` command can be provided through the [MinGW](https://www.mingw-w64.org/)

During the MinGW installation `posix` should be selected as a `threads` module.

### MacOS

 1. `make` command can be provided through the `brew install make`
 2. `cmake` command can be provided through the `brew install cmake`
 3. `g++` command should be available after XCode is installed or throught the `brew install gcc`

### Linux

 1. `make` command can be provided through the `brew install make`
 2. `cmake` command can be provided through the both a `apt install cmake` or a `brew install cmake`
 3. `g++` command can be provided through the `apt-get install g++-version`


## Build
1. Build all the dependencies through the appropriate batch file (`depsWin.bat` or `depsUnix.sh` depending on your OS) from root folder of the project.
2. Build main app by running `make` or by entering into "build/" subfolder and running from there `cmake ..` 


## CLI Options

| Command         | Reuired                | Initial value | Description                                                         |
|-----------------|------------------------|---------------|---------------------------------------------------------------------|
| -h, --help      | No                     |               | List all of the available commands for the current program version. |
| -i, --input     | Yes :white_check_mark: |               | Input model path                                                    |
| -o, --output    | No                     | ./exported    | Output directory                                                    |
| -l, --limit     | No                     | 2048          | Polygons limit (only for `Voxel, Regular` split alorithms)          |
| -g, --grid      | No                     | 64            | Grid resolution (only for `Voxel` algorithm)                        |
| --iso           | No                     |               | Currently unused                                                    |
| -a, --algorithm | No                     | voxel         | Split algorithm to use                                              |
| --algorithms    | No                     |               | Available algorithms list                                           |
| -f, --format    | No                     | b3dm          | Model format to export                                              |
| --formats       | No                     |               | Available formats list                                              |
| --compress      | No                     |               | Enables Draco compressing                                           |
| --texlevels     | No                     | 8             | Number of texture LOD levels (0 - disables texture LOD generation)  |

### -h, --help
Prints an application help message into the CLI.

### -i, --input (<font size="2">`required`</font>)
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

### --compress
Enables Draco compression

***Example***
 > 3dtg ./someFolder/myModel.obj ./outdir --compress

### --texlevels
Texture LOD levels count

Pass 0 if you want to disable texture LOD generation.

Default value is `8`

***Example***
 > 3dtg ./someFolder/myModel.obj ./outdir --texlevels 4


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