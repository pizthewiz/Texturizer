# Texturizer
A simple Cinder application to demonstrate image conversion to a DXT1 or DXT5 compressed DDS file.

[S3 Texture Compression](http://en.wikipedia.org/wiki/S3_Texture_Compression) (of which DXT1 and DXT5 are two of several variations) is a lossy compression format which many desktop GPUs support via the [`GL_EXT_texture_compression_s3tc`](https://www.opengl.org/registry/specs/EXT/texture_compression_s3tc.txt) extension - this allows the compressed image to be transferred directly to the GPU and then decompressed in hardware rather than the more costly transfer of a decompressed image from CPU to GPU.

Thus far, the application has only been tested on OS X 10.10.2 and does require the [glNext branch](https://github.com/cinder/Cinder/tree/glNext) of Cinder. While it should run on Windows, there are several existing tools on the platform that provide more compression variants and features, see NVIDIA's [Legacy Texture Tools](https://developer.nvidia.com/legacy-texture-tools) and ATI/AMD's [The Compressonator](http://developer.amd.com/tools-and-sdks/archive/legacy-cpu-gpu-tools/the-compressonator/).

DXT1 and DXT5 compression is provided by the great [SOIL2](https://bitbucket.org/SpartanJ/soil2) project and the included image _uvtemplate.bmp_ is from the [textured cube tutorial](http://www.opengl-tutorial.org/beginners-tutorials/tutorial-5-a-textured-cube/) on [opengl-tutorial.org](http://www.opengl-tutorial.org).
