# Texturizer
A simple Cinder application to demonstrate image conversion to a DXT1 or DXT5 compressed DDS file.

[S3 Texture Compression](http://en.wikipedia.org/wiki/S3_Texture_Compression) (of which DXT1 and DXT5 are two of several variations) is a lossy compression format which most desktop GPUs support via the [`GL_EXT_texture_compression_s3tc`](https://www.opengl.org/registry/specs/EXT/texture_compression_s3tc.txt) extension - this allows the compressed image to be transferred directly to the GPU and then decompressed in hardware rather than the more costly transfer of a decompressed image from CPU to GPU.

DXT1 and DXT5 compression is provided by the great [RygsDXTc](https://github.com/Cyan4973/RygsDXTc) project. The included image _uvtemplate.bmp_ is from the [textured cube tutorial](http://www.opengl-tutorial.org/beginners-tutorials/tutorial-5-a-textured-cube/) from [opengl-tutorial.org](http://www.opengl-tutorial.org).
