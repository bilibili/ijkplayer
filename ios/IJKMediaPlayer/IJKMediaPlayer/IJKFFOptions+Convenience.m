/*
 * IJKFFOptions+Convenience.m
 *
 * Copyright (c) 2015 Zhang Rui <bbcallen@gmail.com>
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

#import "IJKFFOptions+Convenience.h"

@implementation IJKFFOptions (IJKFFOptions_Convenience)

+ (IJKFFOptions *)optionsByDefault
{
    IJKFFOptions *options = [[IJKFFOptions alloc] init];

    [options setMaxFps:30];
    [options setFrameDrop:0];
    [options setVideoPictureSize:0];
    [options setVideoToolboxMaxFrameWidth:960];

    [options setReconnect:1];
    [options setTimeout:30 * 1000 * 1000];
    [options setUserAgent:@"ijkplayer"];

    [options setSkipLoopFilter:IJK_AVDISCARD_ALL];
    [options setSkipFrame:IJK_AVDISCARD_ALL];

    return options;
}

#pragma mark Common Helper

-(void)setFormatOptionValue:(NSString *)value forKey:(NSString *)key
{
    [self setOptionValue:value forKey:key ofCategory:kIJKFFOptionCategoryFormat];
}

-(void)setCodecOptionValue:(NSString *)value forKey:(NSString *)key
{
    [self setOptionValue:value forKey:key ofCategory:kIJKFFOptionCategoryCodec];
}

-(void)setSwsOptionValue:(NSString *)value forKey:(NSString *)key
{
    [self setOptionValue:value forKey:key ofCategory:kIJKFFOptionCategorySws];
}

-(void)setPlayerOptionValue:(NSString *)value forKey:(NSString *)key
{
    [self setOptionValue:value forKey:key ofCategory:kIJKFFOptionCategoryPlayer];
}

-(void)setFormatOptionIntValue:(int64_t)value forKey:(NSString *)key
{
    [self setOptionIntValue:value forKey:key ofCategory:kIJKFFOptionCategoryFormat];
}

-(void)setCodecOptionIntValue:(int64_t)value forKey:(NSString *)key
{
    [self setOptionIntValue:value forKey:key ofCategory:kIJKFFOptionCategoryCodec];
}

-(void)setSwsOptionIntValue:(int64_t)value forKey:(NSString *)key
{
    [self setOptionIntValue:value forKey:key ofCategory:kIJKFFOptionCategorySws];
}

-(void)setPlayerOptionIntValue:(int64_t)value forKey:(NSString *)key
{
    [self setOptionIntValue:value forKey:key ofCategory:kIJKFFOptionCategoryPlayer];
}

-(void)setMaxBufferSize:(int)maxBufferSize
{
    [self setPlayerOptionIntValue:maxBufferSize forKey:@"max-buffer-size"];
}


#pragma mark Player options

-(void)setMaxFps:(int)value
{
    [self setPlayerOptionIntValue:value forKey:@"max-fps"];
}

-(void)setFrameDrop:(int)value
{
    [self setPlayerOptionIntValue:value forKey:@"framedrop"];
}

-(void)setVideoPictureSize:(int)value
{
    [self setPlayerOptionIntValue:value forKey:@"video-pictq-size"];
}

-(void)setVideoToolboxEnabled:(BOOL)value
{
    [self setPlayerOptionIntValue:(value ? 1 : 0) forKey:@"videotoolbox"];
}

-(void)setVideoToolboxMaxFrameWidth:(int)value
{
    [self setPlayerOptionIntValue:value forKey:@"videotoolbox-max-frame-width"];
}


#pragma mark Format options

-(void)setReconnect:(int)value
{
    [self setFormatOptionIntValue:value forKey:@"reconnect"];
}

-(void)setTimeout:(int64_t)value
{
    [self setFormatOptionIntValue:value forKey:@"timeout"];
}

-(void)setUserAgent:(NSString *)value
{
    [self setFormatOptionValue:value forKey:@"user-agent"];
}


#pragma mark Codec options

-(void)setSkipLoopFilter:(IJKAVDiscard)value
{
    [self setCodecOptionIntValue:value forKey:@"skip_loop_filter"];
}

-(void)setSkipFrame:(IJKAVDiscard)value
{
    [self setCodecOptionIntValue:value forKey:@"skip_frame"];
}

@end
