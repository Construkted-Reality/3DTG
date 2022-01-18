## Usage
The syntax is:
3dtg.exe -i path\to\input\obj path\to\3d-tile\folder

## Memory usage
A rough estimate of memory usage can be calculated by adding up the number of textures and the size of the uncompressed 3d mesh.
16k textures use about 1GB ram
8k textures use about 0.25 GB of ram (256MB)
4k textures use about 0.06 GB ram (64MB)

System ram usage = (Mesh_in_OBJ_format) + (#_of_textures * Texture_Resolution)

This will provide a rough ballpark on the amount of system ram required for processing your textured mesh.
