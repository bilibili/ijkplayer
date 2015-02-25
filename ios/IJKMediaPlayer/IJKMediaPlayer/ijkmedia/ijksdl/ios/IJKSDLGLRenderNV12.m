/*
 * IJKSDLGLRenderNV12.m
 *
 * Copyright (c) 2014 Zhou Quan <bbcallen@gmail.com>
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

#import "IJKSDLGLRenderNV12.h"
#import "IJKSDLGLShader.h"

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
    1.164,  1.164, 1.164,
    0.0, -0.213, 2.112,
    1.793, -0.533,   0.0,
};





@implementation IJKSDLGLRenderNV12 {
    GLint _uniform[1];
    GLint _uniformSamplers[2];
    GLuint _textures[2];
}

- (BOOL) isValid
{
    return (_textures[0] != 0);
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

- (void) display: (SDL_VoutOverlay *) overlay
{
    assert(overlay->planes);
    assert(overlay->format == SDL_FCC_NV12);
    assert(overlay->planes == 2);

    const NSUInteger frameHeight = overlay->h;

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    if (0 == _textures[0])
        glGenTextures(2, _textures);

    const UInt8 *pixels[2] = { overlay->pixels[0], overlay->pixels[1] };
    const NSUInteger widths[2]  = { overlay->pitches[0], overlay->pitches[1]/2 };
    const NSUInteger heights[2] = { frameHeight, frameHeight / 2 };


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
    if (_textures[0])
        glDeleteTextures(2, _textures);
}

@end
