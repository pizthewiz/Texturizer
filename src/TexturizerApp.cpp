//
//  TexturizerApp.cpp
//  TexturizerApp
//
//  Created by Jean-Pierre Mouilleseaux on 02 Feb 2015.
//  Copyright (c) 2015 Chorded Constructions. All rights reserved.
//

#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/Texture.h"

extern "C" {
    #include "image_DXT.h"
}

using namespace ci;
using namespace ci::app;
using namespace std;

#define DXT1 1

class TexturizerApp : public AppNative {
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
        mSourceSurface = Surface::create(loadImage(loadAsset("uvtemplate.bmp")));
    } catch (...) {
        console() << "unable to create surface" << endl;
        quit();
    }

    // create intermediate when necessary
#if defined(DXT1)
    SurfaceChannelOrder channelOrder = SurfaceChannelOrder::RGB;
#else
    SurfaceChannelOrder channelOrder = SurfaceChannelOrder::RGBA;
#endif
    if (mSourceSurface->getChannelOrder().getCode() != channelOrder.getCode()) {
        int32_t width = mSourceSurface->getWidth();
        int32_t height = mSourceSurface->getHeight();
#if defined(DXT1)
        int32_t rowBytes = 3;
#else
        int32_t rowBytes = 4;
#endif
        size_t dataSize = width * height * rowBytes;
        unsigned char* intermediate = (unsigned char*)malloc(dataSize);
        size_t offset = 0;

        Surface::Iter iter = mSourceSurface->getIter();
        while (iter.line()) {
            while (iter.pixel()) {
                intermediate[offset++] = iter.r();
                intermediate[offset++] = iter.g();
                intermediate[offset++] = iter.b();
#if !defined(DXT1)
                intermediate[offset++] = mSourceSurface->hasAlpha() ? iter.a() : 255;
#endif
            }
        }

        try {
            mIntermediateSurface = Surface::create(intermediate, width, height, rowBytes, channelOrder);
        } catch (...) {
            console() << "failed to create intermediate RGBA surface" << endl;
            quit();
        }
    }

    Surface8uRef s = mIntermediateSurface ? mIntermediateSurface : mSourceSurface;
    unsigned char* source = s->getData();
    int width = s->getWidth();
    int height = s->getHeight();

    int dataSize = 0;
#if defined(DXT1)
    unsigned char* destination = convert_image_to_DXT1(source, width, height, 3, &dataSize);
#else
    unsigned char* destination = convert_image_to_DXT5(source, width, height, 4, &dataSize);
#endif

    DDS_header* header = (DDS_header*)calloc(4, sizeof(DDS_header));
    header->dwMagic = ('D' << 0) | ('D' << 8) | ('S' << 16) | (' ' << 24);
    header->dwSize = 124;
    header->dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_LINEARSIZE;
    header->dwHeight = height;
    header->dwWidth = width;
    header->dwPitchOrLinearSize = dataSize;
    header->sPixelFormat.dwSize = 32;
    header->sPixelFormat.dwFlags = DDPF_FOURCC;
#if defined(DXT1)
    header->sPixelFormat.dwFourCC = ('D' << 0) | ('X' << 8) | ('T' << 16) | ('1' << 24);
#else
    header->sPixelFormat.dwFourCC = ('D' << 0) | ('X' << 8) | ('T' << 16) | ('5' << 24);
#endif
    header->sCaps.dwCaps1 = DDSCAPS_TEXTURE;

    Buffer buffer(sizeof(DDS_header) + dataSize);
    memcpy((unsigned char*)buffer.getData(), header, sizeof(DDS_header));
    memcpy((unsigned char*)buffer.getData() + sizeof(DDS_header), destination, dataSize);
    free(destination);

    fs::path ddsFilePath = getAssetPath("") / "out.DDS";
    buffer.write(DataTargetPath::createRef(ddsFilePath));

    try {
        mTexture = gl::Texture::createFromDds(DataSourceBuffer::create(buffer));
    } catch (...) {
        console() << "failed to create texture from DDS file" << endl;
        quit();
    }
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

CINDER_APP_NATIVE(TexturizerApp, RendererGl)
