/*
 * IJKSDLGLRenderI420.m
 *
 * Copyright (c) 2013 Zhang Rui <bbcallen@gmail.com>
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

#import "IJKSDLGLRenderI420.h"
#import "IJKSDLGLShader.h"

static NSString *const g_yuvFragmentShaderString = IJK_SHADER_STRING
(
    varying highp vec2 v_texcoord;
    uniform sampler2D s_texture_y;
    uniform sampler2D s_texture_u;
    uniform sampler2D s_texture_v;

    void main()
    {
        highp float y = texture2D(s_texture_y, v_texcoord).r;
        highp float u = texture2D(s_texture_u, v_texcoord).r - 0.5;
        highp float v = texture2D(s_texture_v, v_texcoord).r - 0.5;

        highp float r = y +               1.40200 * v;
        highp float g = y - 0.34414 * u - 0.71414 * v;
        highp float b = y + 1.77200 * u;

        gl_FragColor = vec4(r,g,b,1.0);
    }
);

@implementation IJKSDLGLRenderI420 {
    GLint _uniformSamplers[3];
    GLuint _textures[3];
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
    _uniformSamplers[0] = glGetUniformLocation(program, "s_texture_y");
    _uniformSamplers[1] = glGetUniformLocation(program, "s_texture_u");
    _uniformSamplers[2] = glGetUniformLocation(program, "s_texture_v");
}

- (void) render: (SDL_VoutOverlay *) overlay
{
    assert(overlay->planes);
    assert(overlay->format == SDL_FCC_I420);
    assert(overlay->planes == 3);
    assert(overlay->planes == 3);
    // assert(yuvFrame.luma.length == yuvFrame.width * yuvFrame.height);
    // assert(yuvFrame.chromaB.length == (yuvFrame.width * yuvFrame.height) / 4);
    // assert(yuvFrame.chromaR.length == (yuvFrame.width * yuvFrame.height) / 4);

    const NSUInteger frameHeight = overlay->h;

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    if (0 == _textures[0])
        glGenTextures(3, _textures);

    const UInt8 *pixels[3] = { overlay->pixels[0], overlay->pixels[1], overlay->pixels[2] };
    const NSUInteger widths[3]  = { overlay->pitches[0], overlay->pitches[1], overlay->pitches[2] };
    const NSUInteger heights[3] = { frameHeight, frameHeight / 2, frameHeight / 2 };

    for (int i = 0; i < 3; ++i) {

        glBindTexture(GL_TEXTURE_2D, _textures[i]);

        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     GL_LUMINANCE,
                     (int)widths[i],
                     (int)heights[i],
                     0,
                     GL_LUMINANCE,
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

    return YES;
}

- (void) dealloc
{
    if (_textures[0])
        glDeleteTextures(3, _textures);
}

@end
