//
//  IJKSDLGLShader.m
//  IJKMediaPlayer
//
//  Created by ZhangRui on 13-9-24.
//  Copyright (c) 2013年 bilibili. All rights reserved.
//

#import "IJKSDLGLShader.h"

//////////////////////////////////////////////////////////

#pragma mark - shaders

NSString *const rgbFragmentShaderString = IJK_SHADER_STRING
(
    varying highp vec2 v_texcoord;
    uniform sampler2D s_texture;

    void main()
    {
        gl_FragColor = texture2D(s_texture, v_texcoord);
    }
);
