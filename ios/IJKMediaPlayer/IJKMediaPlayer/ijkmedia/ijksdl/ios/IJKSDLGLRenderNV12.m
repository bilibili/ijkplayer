/*
 * IJKSDLGLRenderNV12.m
 *
 * Copyright (c) 2014 Zhou Quan <zhouqicy@gmail.com>
 *
 * This file is part of ijkPlayer.
 *
 * ijkPlayer is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * ijkPlayer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with ijkPlayer; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#import "IJKSDLGLRenderNV12.h"
#import "IJKSDLGLShader.h"
#include "ijksdl_vout_overlay_videotoolbox.h"

static NSString *const g_nv12FragmentShaderString = IJK_SHADER_STRING
(
    varying highp vec2 v_texcoord;
    precision mediump float;
    uniform sampler2D SamplerY;
    uniform sampler2D SamplerUV;
    uniform mat3 colorConversionMatrix;

    void main()
    {
        mediump vec3 yuv;
        lowp vec3 rgb;

        // Subtract constants to map the video range start at 0
        yuv.x = (texture2D(SamplerY, v_texcoord).r - (16.0/255.0));
        yuv.yz = (texture2D(SamplerUV, v_texcoord).rg - vec2(0.5, 0.5));
        rgb = colorConversionMatrix * yuv;
        gl_FragColor = vec4(rgb,1);
    }
);

// BT.601, which is the standard for SDTV.
static const GLfloat kColorConversion601[] = {
    1.164,  1.164, 1.164,
    0.0,   -0.392, 2.017,
    1.596, -0.813,   0.0,
};

// BT.709, which is the standard for HDTV.
static const GLfloat kColorConversion709[] = {
    1.164,  1.164,  1.164,
    0.0,   -0.213,  2.112,
    1.793, -0.533,  0.0,
};





@implementation IJKSDLGLRenderNV12 {
    GLint _uniform[1];
    GLint _uniformSamplers[2];
    GLuint _textures[2];

    CVOpenGLESTextureCacheRef _textureCache;
    CVOpenGLESTextureRef      _cvTexturesRef[2];

    const GLfloat *_preferredConversion;
}

-(id)initWithTextureCache:(CVOpenGLESTextureCacheRef) textureCache
{
    self = [super init];
    if (self) {
        _textureCache = textureCache;
    }
    return self;
}

- (BOOL) isValid
{
    return (_textures[0] != 0) && (_textures[1] != 0);
}

- (NSString *) fragmentShader
{
    return g_nv12FragmentShaderString;
}

- (void) resolveUniforms: (GLuint) program
{
    _uniformSamplers[0] = glGetUniformLocation(program, "SamplerY");
    _uniformSamplers[1] = glGetUniformLocation(program, "SamplerUV");
    _uniform[0] = glGetUniformLocation(program, "colorConversionMatrix");
}

- (void) render: (SDL_VoutOverlay *) overlay
{
    assert(overlay->planes);
    assert(overlay->format == SDL_FCC_NV12);
    assert(overlay->planes == 2);

    if (!overlay->is_private)
        return;

    if (!_textureCache) {
        ALOGE("nil textureCache\n");
        return;
    }

    CVPixelBufferRef pixelBuffer = SDL_VoutOverlayVideoToolBox_GetCVPixelBufferRef(overlay);
    if (!pixelBuffer) {
        ALOGE("nil pixelBuffer in overlay\n");
        return;
    }

    CFTypeRef colorAttachments = CVBufferGetAttachment(pixelBuffer, kCVImageBufferYCbCrMatrixKey, NULL);
    if (colorAttachments == kCVImageBufferYCbCrMatrix_ITU_R_601_4) {
        _preferredConversion = kColorConversion601;
    } else if (colorAttachments == kCVImageBufferYCbCrMatrix_ITU_R_709_2){
        _preferredConversion = kColorConversion709;
    } else {
        _preferredConversion = kColorConversion709;
    }

    for (int i = 0; i < 2; ++i) {
        if (_cvTexturesRef[i]) {
            CFRelease(_cvTexturesRef[i]);
            _cvTexturesRef[i] = 0;
            _textures[i] = 0;
        }
    }

    // Periodic texture cache flush every frame
    if (_textureCache)
        CVOpenGLESTextureCacheFlush(_textureCache, 0);

    if (_textures[0])
        glDeleteTextures(2, _textures);

    size_t frameWidth  = CVPixelBufferGetWidth(pixelBuffer);
    size_t frameHeight = CVPixelBufferGetHeight(pixelBuffer);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    CVOpenGLESTextureCacheCreateTextureFromImage(kCFAllocatorDefault,
                                                 _textureCache,
                                                 pixelBuffer,
                                                 NULL,
                                                 GL_TEXTURE_2D,
                                                 GL_RED_EXT,
                                                 (GLsizei)frameWidth,
                                                 (GLsizei)frameHeight,
                                                 GL_RED_EXT,
                                                 GL_UNSIGNED_BYTE,
                                                 0,
                                                 &_cvTexturesRef[0]);
    _textures[0] = CVOpenGLESTextureGetName(_cvTexturesRef[0]);
    glBindTexture(CVOpenGLESTextureGetTarget(_cvTexturesRef[0]), _textures[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


    CVOpenGLESTextureCacheCreateTextureFromImage(kCFAllocatorDefault,
                                                 _textureCache,
                                                 pixelBuffer,
                                                 NULL,
                                                 GL_TEXTURE_2D,
                                                 GL_RG_EXT,
                                                 (GLsizei)frameWidth / 2,
                                                 (GLsizei)frameHeight / 2,
                                                 GL_RG_EXT,
                                                 GL_UNSIGNED_BYTE,
                                                 1,
                                                 &_cvTexturesRef[1]);
    _textures[1] = CVOpenGLESTextureGetName(_cvTexturesRef[1]);
    glBindTexture(CVOpenGLESTextureGetTarget(_cvTexturesRef[1]), _textures[1]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

- (BOOL) prepareDisplay
{
    if (_textures[0] == 0)
        return NO;

    for (int i = 0; i < 2; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, _textures[i]);
        glUniform1i(_uniformSamplers[i], i);
    }

    glUniformMatrix3fv(_uniform[0], 1, GL_FALSE, _preferredConversion);
    return YES;
}

- (void) dealloc
{
    for (int i = 0; i < 2; ++i) {
        if (_cvTexturesRef[i]) {
            CFRelease(_cvTexturesRef[i]);
            _cvTexturesRef[i] = 0;
            _textures[i] = 0;
        }
    }

    if (_textures[0])
        glDeleteTextures(2, _textures);
}

@end
