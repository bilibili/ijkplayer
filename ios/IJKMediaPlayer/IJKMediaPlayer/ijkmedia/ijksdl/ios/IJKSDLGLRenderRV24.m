/*
 * IJKSDLGLRenderRV24.m
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

#import "IJKSDLGLRenderRV24.h"
#import "IJKSDLGLShader.h"

static NSString *const g_rgbFragmentShaderString = IJK_SHADER_STRING
(
    varying highp vec2 v_texcoord;
    uniform sampler2D s_texture;

    void main()
    {
        gl_FragColor = texture2D(s_texture, v_texcoord);
    }
);

@implementation IJKSDLGLRenderRV24 {
    GLint _uniformSamplers[1];
    GLuint _textures[1];
}

- (BOOL) isValid
{
    return (_textures[0] != 0);
}

- (NSString *) fragmentShader
{
    return g_rgbFragmentShaderString;
}

- (void) resolveUniforms: (GLuint) program
{
    _uniformSamplers[0] = glGetUniformLocation(program, "s_texture");
}

- (void) render: (SDL_VoutOverlay *) overlay
{
    assert(overlay->planes);
    assert(overlay->format == SDL_FCC_RV24);
    assert(overlay->planes == 1);
    assert(overlay->planes == 1);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    if (0 == _textures[0])
        glGenTextures(1, _textures);

    const UInt8 *pixels[1] = { overlay->pixels[0] };
    const NSUInteger widths[1]  = { overlay->pitches[0] / 3};
    const NSUInteger heights[1] = { overlay->h };

    for (int i = 0; i < 1; ++i) {

        glBindTexture(GL_TEXTURE_2D, _textures[i]);

        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     GL_RGB,
                     (int)widths[i],
                     (int)heights[i],
                     0,
                     GL_RGB,
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

    for (int i = 0; i < 1; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, _textures[i]);
        glUniform1i(_uniformSamplers[i], i);
    }

    return YES;
}

- (void) dealloc
{
    if (_textures[0])
        glDeleteTextures(1, _textures);
}

@end
