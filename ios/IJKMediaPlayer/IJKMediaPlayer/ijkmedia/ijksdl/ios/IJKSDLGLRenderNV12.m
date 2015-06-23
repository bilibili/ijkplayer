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

//// BT.601, which is the standard for SDTV.
//static const GLfloat kColorConversion601[] = {
//    1.164,  1.164, 1.164,
//    0.0, -0.392, 2.017,
//    1.596, -0.813,   0.0,
//};

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

- (void) displayInternal:(SDL_VoutOverlay *)overlay withPixelBuffer:(CVPixelBufferRef) pixelBuffer
{
    assert(overlay->planes);
    assert(overlay->format == SDL_FCC_NV12);
    assert(overlay->planes == 2);

    if (overlay->pixels[0] == NULL || overlay->pixels[1] == NULL)
        return;

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

- (void) displayInternal:(SDL_VoutOverlay *) overlay
{
    assert(overlay->planes);
    assert(overlay->format == SDL_FCC_NV12);
    assert(overlay->planes == 2);

    if (overlay->pixels[0] == NULL || overlay->pixels[1] == NULL)
        return;

    const NSUInteger frameHeight = overlay->h;

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    if (0 == _textures[0])
        glGenTextures(2, _textures);

    const UInt8 *pixels[2] = { overlay->pixels[0], overlay->pixels[1] };
    const size_t widths[2]  = { overlay->pitches[0], overlay->pitches[1]/2 };
    const size_t heights[2] = { frameHeight, frameHeight / 2 };


    glBindTexture(GL_TEXTURE_2D, _textures[0]);
    glTexImage2D(GL_TEXTURE_2D,
                     0,
                     GL_RED_EXT,
                     (int)widths[0],
                     (int)heights[0],
                     0,
                     GL_RED_EXT,
                     GL_UNSIGNED_BYTE,
                     pixels[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


    glBindTexture(GL_TEXTURE_2D, _textures[1]);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RG_EXT,
                 (int)widths[1],
                 (int)heights[1],
                 0,
                 GL_RG_EXT,
                 GL_UNSIGNED_BYTE,
                 pixels[1]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

- (void) render: (SDL_VoutOverlay *) overlay
{
    assert(overlay->planes);
    assert(overlay->format == SDL_FCC_NV12);
    assert(overlay->planes == 2);

    CVPixelBufferRef pixelBuffer = nil;
    if (_textureCache)
        pixelBuffer = SDL_VoutOverlayVideoToolBox_GetCVPixelBufferRef(overlay);

    if (pixelBuffer) {
        [self displayInternal:overlay withPixelBuffer:pixelBuffer];
    } else {
        [self displayInternal:overlay];
    }
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

    glUniformMatrix3fv(_uniform[0], 1, GL_FALSE, kColorConversion709);
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
