//
//  IJKSDLGLRenderI420.m
//  IJKMediaPlayer
//
//  Created by ZhangRui on 13-9-24.
//  Copyright (c) 2013年 bilibili. All rights reserved.
//

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

        highp float r = y +             1.402 * v;
        highp float g = y - 0.344 * u - 0.714 * v;
        highp float b = y + 1.772 * u;

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

- (void) display: (SDL_VoutOverlay *) overlay
{
    assert(overlay->planes);
    assert(overlay->format == SDL_FCC_I420);
    assert(overlay->planes == 3);
    assert(overlay->planes == 3);
    // assert(yuvFrame.luma.length == yuvFrame.width * yuvFrame.height);
    // assert(yuvFrame.chromaB.length == (yuvFrame.width * yuvFrame.height) / 4);
    // assert(yuvFrame.chromaR.length == (yuvFrame.width * yuvFrame.height) / 4);

    const NSUInteger frameWidth = overlay->w;
    const NSUInteger frameHeight = overlay->h;

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    if (0 == _textures[0])
        glGenTextures(3, _textures);

    const UInt8 *pixels[3] = {overlay->pixels[0], overlay->pixels[1], overlay->pixels[2]};
    const NSUInteger widths[3]  = { frameWidth, frameWidth / 2, frameWidth / 2 };
    const NSUInteger heights[3] = { frameHeight, frameHeight / 2, frameHeight / 2 };

    for (int i = 0; i < 3; ++i) {

        glBindTexture(GL_TEXTURE_2D, _textures[i]);

        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     GL_LUMINANCE,
                     widths[i],
                     heights[i],
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
