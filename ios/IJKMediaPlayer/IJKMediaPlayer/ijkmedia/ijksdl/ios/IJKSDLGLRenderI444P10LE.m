/*
 * IJKSDLGLRenderI444P10LE.m
 *
 * Copyright (c) 2015 Zhang Rui <bbcallen@gmail.com>
 *
 * based on https://github.com/kolyvan/kxmovie
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

#import "IJKSDLGLRenderI444P10LE.h"
#import "IJKSDLGLShader.h"

static NSString *const g_yuvFragmentShaderString = IJK_SHADER_STRING
(
    varying highp vec2 v_texcoord;
    precision mediump float;
    uniform sampler2D SamplerY;
    uniform sampler2D SamplerU;
    uniform sampler2D SamplerV;
    uniform mat3 colorConversionMatrix;

    void main()
    {
        highp vec3 yuvr;
        highp vec3 yuvg;
        highp vec3 yuv;
        lowp vec3 rgb;

        // Subtract constants to map the video range start at 0
        yuvr.x = texture2D(SamplerY, v_texcoord).r;
        yuvr.y = texture2D(SamplerU, v_texcoord).r;
        yuvr.z = texture2D(SamplerV, v_texcoord).r;
        yuvg.x = texture2D(SamplerY, v_texcoord).g;
        yuvg.y = texture2D(SamplerU, v_texcoord).g;
        yuvg.z = texture2D(SamplerV, v_texcoord).g;

        yuv    = (yuvr * 255.0 + yuvg * 255.0 * 256.0) / (1023.0);
        yuv.x  = yuv.x - 16.0 / 255.0;
        yuv.yz = yuv.yz - vec2(0.5, 0.5);

        rgb = colorConversionMatrix * yuv;
        gl_FragColor = vec4(rgb,1);
    }
);

@implementation IJKSDLGLRenderI444P10LE {
    GLint _uniform[1];
    GLint _uniformSamplers[3];
    GLuint _textures[3];

    const GLfloat *_preferredConversion;
}

- (BOOL) isValid
{
    return (_textures[0] != 0);
}

- (NSString *) fragmentShader
{
    return g_yuvFragmentShaderString;
}

- (void) resolveUniforms: (GLuint) program
{
    _uniformSamplers[0] = glGetUniformLocation(program, "SamplerY");
    _uniformSamplers[1] = glGetUniformLocation(program, "SamplerU");
    _uniformSamplers[2] = glGetUniformLocation(program, "SamplerV");
    _uniform[0] = glGetUniformLocation(program, "colorConversionMatrix");
}

- (void) render: (SDL_VoutOverlay *) overlay
{
    assert(overlay->planes);
    assert(overlay->format == SDL_FCC_I444P10LE);
    assert(overlay->planes == 3);
    // assert(yuvFrame.luma.length == yuvFrame.width * yuvFrame.height);
    // assert(yuvFrame.chromaB.length == (yuvFrame.width * yuvFrame.height) / 4);
    // assert(yuvFrame.chromaR.length == (yuvFrame.width * yuvFrame.height) / 4);

    const NSUInteger frameHeight = overlay->h;

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    if (0 == _textures[0])
        glGenTextures(3, _textures);

    _preferredConversion = kColorConversion709;

    const UInt8 *pixels[3] = { overlay->pixels[0], overlay->pixels[1], overlay->pixels[2] };
    const NSUInteger widths[3]  = { overlay->pitches[0] / 2, overlay->pitches[1] / 2, overlay->pitches[2] / 2 };
    const NSUInteger heights[3] = { frameHeight, frameHeight, frameHeight };

    for (int i = 0; i < 3; ++i) {

        glBindTexture(GL_TEXTURE_2D, _textures[i]);

        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     GL_RG_EXT,
                     (int)widths[i],
                     (int)heights[i],
                     0,
                     GL_RG_EXT,
                     GL_UNSIGNED_BYTE,
                     pixels[i]);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
}

- (BOOL) prepareDisplay
{
    if (_textures[0] == 0)
        return NO;

    for (int i = 0; i < 3; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, _textures[i]);
        glUniform1i(_uniformSamplers[i], i);
    }

    glUniformMatrix3fv(_uniform[0], 1, GL_FALSE, _preferredConversion);
    return YES;
}

- (void) dealloc
{
    if (_textures[0])
        glDeleteTextures(3, _textures);
}

@end
