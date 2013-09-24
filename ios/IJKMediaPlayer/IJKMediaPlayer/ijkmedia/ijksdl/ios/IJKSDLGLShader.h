//
//  IJKSDLGLShader.h
//  IJKMediaPlayer
//
//  Created by ZhangRui on 13-9-24.
//  Copyright (c) 2013年 bilibili. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface IJKSDLGLShader : NSObject

@end

#define IJK_STRINGIZE(x) #x
#define IJK_STRINGIZE2(x) IJK_STRINGIZE(x)
#define IJK_SHADER_STRING(text) @ IJK_STRINGIZE2(text)

typedef enum {
    KxVideoFrameFormatRGB,
    KxVideoFrameFormatYUV,
} KxVideoFrameFormat;

@interface KxVideoFrame : NSObject
@property (readonly, nonatomic) CGFloat position;
@property (readonly, nonatomic) CGFloat duration;
@property (readwrite, nonatomic) NSUInteger width;
@property (readwrite, nonatomic) NSUInteger height;

@property (readonly, nonatomic) KxVideoFrameFormat format;
@end
