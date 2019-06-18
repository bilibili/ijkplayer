/*
 * IJKFFMoviePlayerDef.m
 *
 * Copyright (c) 2019 Befovy <befovy@gmail.com>
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

#import <Foundation/Foundation.h>
#import "IJKFFOptions.h"

NS_ASSUME_NONNULL_BEGIN

@interface IJKFFMediaPlayer : NSObject


- (IJKFFMediaPlayer *)init;

@property (readonly, nonatomic) int videoWidth;
@property (readonly, nonatomic) int videoHeight;
@property (readonly, nonatomic) int videoSarNum;
@property (readonly, nonatomic) int videoSarDen;

- (void) setDataSource:(NSString *)url;
- (void) prepareAsync;
- (void) start;
- (void) stop;
- (void) pause;
- (BOOL) isPlaying;
- (void) shutdown;

- (void)setOptionValue:(NSString *)value
                forKey:(NSString *)key
            ofCategory:(IJKFFOptionCategory)category;

- (void)setOptionIntValue:(int64_t)value
                   forKey:(NSString *)key
               ofCategory:(IJKFFOptionCategory)category;

@end

NS_ASSUME_NONNULL_END
