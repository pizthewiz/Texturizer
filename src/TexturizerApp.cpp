//
//  TexturizerApp.cpp
//  TexturizerApp
//
//  Created by Jean-Pierre Mouilleseaux on 02 Feb 2015.
//  Copyright (c) 2015 Chorded Constructions. All rights reserved.
//

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/Texture.h"

#include "stb_dxt.h"

using namespace ci;
using namespace ci::app;
using namespace std;

//#define DXT1 1
#define DXT5 1

class TexturizerApp : public App {
public:
    void setup() override;
    void quit() override;
    void update() override;
    void draw() override;

private:
    Surface8uRef mSourceSurface;
    Surface8uRef mIntermediateSurface;
    gl::TextureRef mTexture;
};

void TexturizerApp::setup() {
    if (!gl::isExtensionAvailable("GL_EXT_texture_compression_s3tc")) {
        console() << "GPU does not support GL_EXT_texture_compression_s3tc" << endl;
        quit();
    }

    try {
        mSourceSurface = Surface::create(loadImage(loadAsset("uvtemplate.png")));
    } catch (...) {
        console() << "unable to create surface" << endl;
        quit();
    }

    // NB - Much easier via Cinder-DDS: https://github.com/pizthewiz/Cinder-DDS

    // create RGBA intermediate when necessary
    if (mSourceSurface->getChannelOrder().getCode() != SurfaceChannelOrder::RGBA) {
        int32_t width = mSourceSurface->getWidth();
        int32_t height = mSourceSurface->getHeight();
        size_t size = width * height * 4;
        unsigned char* intermediate = (unsigned char*)malloc(size);
        size_t offset = 0;

        Surface::Iter iter = mSourceSurface->getIter();
        while(iter.line()) {
            while(iter.pixel()) {
                intermediate[offset++] = iter.r();
                intermediate[offset++] = iter.g();
                intermediate[offset++] = iter.b();
                intermediate[offset++] = mSourceSurface->hasAlpha() ? iter.a() : 255;
           }
        }

        try {
            mIntermediateSurface = Surface::create(intermediate, width, height, 4, SurfaceChannelOrder::RGBA);
        } catch (...) {
            console() << "failed to create intermediate RGBA surface" << endl;
            quit();
        }
    }

    Surface8uRef s = mIntermediateSurface ? mIntermediateSurface : mSourceSurface;
    unsigned char* source = s->getData();
    int32_t width = s->getWidth();
    int32_t height = s->getHeight();
    size_t dataSize = s->getPixelBytes() * width * height;
#if defined(DXT1)
    dataSize /= 8;
    int compressionFormat = 0;
#elif defined(DXT5)
    dataSize /= 4;
    int compressionFormat = 1;
#endif
    unsigned char* destination = (unsigned char*)malloc(dataSize);

    rygCompress(destination, source, width, height, compressionFormat);

    // lifted from TextureFormatParsers.cpp parseDds
    typedef struct { // DDCOLORKEY
        uint32_t dw1;
        uint32_t dw2;
    } ddColorKey;

    typedef struct  { // DDSCAPS2
        uint32_t dwCaps1;
        uint32_t dwCaps2;
        uint32_t Reserved[2];
    } ddCaps2;

    typedef struct { // DDPIXELFORMAT
        uint32_t  dwSize;
        uint32_t  dwFlags;
        uint32_t  dwFourCC;
        union {
            uint32_t  dwRGBBitCount;
            uint32_t  dwYUVBitCount;
            uint32_t  dwZBufferBitDepth;
            uint32_t  dwAlphaBitDepth;
            uint32_t  dwLuminanceBitCount;
            uint32_t  dwBumpBitCount;
            uint32_t  dwPrivateFormatBitCount;
        } ;
        union {
            uint32_t  dwRBitMask;
            uint32_t  dwYBitMask;
            uint32_t  dwStencilBitDepth;
            uint32_t  dwLuminanceBitMask;
            uint32_t  dwBumpDuBitMask;
            uint32_t  dwOperations;
        } ;
        union {
            uint32_t  dwGBitMask;
            uint32_t  dwUBitMask;
            uint32_t  dwZBitMask;
            uint32_t  dwBumpDvBitMask;
            struct {
                int32_t wFlipMSTypes;
                int32_t wBltMSTypes;
            } MultiSampleCaps;
        };
        union {
            uint32_t  dwBBitMask;
            uint32_t  dwVBitMask;
            uint32_t  dwStencilBitMask;
            uint32_t  dwBumpLuminanceBitMask;
        };
        union {
            uint32_t  dwRGBAlphaBitMask;
            uint32_t  dwYUVAlphaBitMask;
            uint32_t  dwLuminanceAlphaBitMask;
            uint32_t  dwRGBZBitMask;
            uint32_t  dwYUVZBitMask;
        } ;
    } DdPixelFormat;

    typedef struct DdSurface // this is lifted and adapted from DDSURFACEDESC2
    {
        uint32_t               dwSize;                 // size of the DDSURFACEDESC structure
        uint32_t               dwFlags;                // determines what fields are valid
        uint32_t               dwHeight;               // height of surface to be created
        uint32_t               dwWidth;                // width of input surface
        union
        {
            int32_t            lPitch;                 // distance to start of next line (return value only)
            uint32_t           dwLinearSize;           // Formless late-allocated optimized surface size
        };
        union
        {
            uint32_t           dwBackBufferCount;      // number of back buffers requested
            uint32_t           dwDepth;                // the depth if this is a volume texture
        };
        union
        {
            uint32_t			dwMipMapCount;          // number of mip-map levels requestde
                                                    // dwZBufferBitDepth removed, use ddpfPixelFormat one instead
            uint32_t			dwRefreshRate;          // refresh rate (used when display mode is described)
            uint32_t			dwSrcVBHandle;          // The source used in VB::Optimize
        };
        uint32_t				dwAlphaBitDepth;        // depth of alpha buffer requested
        uint32_t				dwReserved;             // reserved
        uint32_t				lpSurface;              // pointer to the associated surface memory
        union
        {
            ddColorKey			ddckCKDestOverlay;      // color key for destination overlay use
            uint32_t			dwEmptyFaceColor;       // Physical color for empty cubemap faces
        };
        ddColorKey          ddckCKDestBlt;          // color key for destination blt use
        ddColorKey          ddckCKSrcOverlay;       // color key for source overlay use
        ddColorKey          ddckCKSrcBlt;           // color key for source blt use
        union
        {
            DdPixelFormat		ddpfPixelFormat;        // pixel format description of the surface
            uint32_t			dwFVF;                  // vertex format description of vertex buffers
        };
        ddCaps2			ddsCaps;                // direct draw surface capabilities
        uint32_t		dwTextureStage;         // stage in multitexture cascade
    } DdSurface;

    enum { FOURCC_DXT1 = 0x31545844, FOURCC_DXT3 = 0x33545844, FOURCC_DXT5 = 0x35545844, FOURCC_DX10 = 0x30315844,
        FOURCC_ATI1 = 0x31495441, FOURCC_ATI2= 0x32495441, DDPF_FOURCC = 0x4 };

    // lifted from SOIL2 image_DXT.h
    /*	The dwFlags member of the original DDSURFACEDESC2 structure
     can be set to one or more of the following values.	*/
#define DDSD_CAPS	0x00000001
#define DDSD_HEIGHT	0x00000002
#define DDSD_WIDTH	0x00000004
#define DDSD_PITCH	0x00000008
#define DDSD_PIXELFORMAT	0x00001000
#define DDSD_MIPMAPCOUNT	0x00020000
#define DDSD_LINEARSIZE	0x00080000
#define DDSD_DEPTH	0x00800000

    /*	The dwCaps1 member of the DDSCAPS2 structure can be
     set to one or more of the following values.	*/
#define DDSCAPS_COMPLEX	0x00000008
#define DDSCAPS_TEXTURE	0x00001000
#define DDSCAPS_MIPMAP	0x00400000

    DdSurface* header = (DdSurface*)calloc(4, sizeof(DdSurface));
    header->dwSize = 124;
    header->dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_LINEARSIZE;
    header->dwHeight = height;
    header->dwWidth = width;
    header->dwLinearSize = dataSize;
    header->ddpfPixelFormat.dwSize = 32;
    header->ddpfPixelFormat.dwFlags = DDPF_FOURCC;
#if defined(DXT1)
    header->ddpfPixelFormat.dwFourCC = FOURCC_DXT1;
#else
    header->ddpfPixelFormat.dwFourCC = FOURCC_DXT5;
#endif
    header->ddsCaps.dwCaps1 = DDSCAPS_TEXTURE;

    Buffer buffer(128 + dataSize);
    memcpy((unsigned char*)buffer.getData() + 0, &"DDS ", 4);
    memcpy((unsigned char*)buffer.getData() + 4, header, 124);
    memcpy((unsigned char*)buffer.getData() + 128, destination, dataSize);
    free(destination);

    fs::path ddsFilePath = getAssetPath("") / "out.DDS";
    buffer.write(DataTargetPath::createRef(ddsFilePath));

    try {
        mTexture = gl::Texture::createFromDds(DataSourceBuffer::create(buffer));
    } catch (...) {
        console() << "failed to create texture from DDS file" << endl;
        quit();
    }

    gl::enableAlphaBlending();
}

void TexturizerApp::quit() {
    // NB: doesn't actually seem to be called
    if (mIntermediateSurface) {
        free(mIntermediateSurface->getData());
    }
}

void TexturizerApp::update() {
}

void TexturizerApp::draw() {
    gl::clear();

    if (!mTexture) {
        return;
    }

    gl::draw(mTexture);
}

CINDER_APP(TexturizerApp, RendererGl)
