/*
 * IJKFFMoviePlayerController.h
 *
 * Copyright (c) 2013 Zhang Rui <bbcallen@gmail.com>
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

#import "IJKMediaPlayback.h"
#import "IJKFFOptions.h"

@interface IJKFFMoviePlayerController : NSObject <IJKMediaPlayback>

- (id)initWithContentURL:(NSURL *)aUrl
             withOptions:(IJKFFOptions *)options;

- (id)initWithContentURL:(NSURL *)aUrl
             withOptions:(IJKFFOptions *)options
     withSegmentResolver:(id<IJKMediaSegmentResolver>)segmentResolver;

- (id)initWithContentURLString:(NSString *)aUrlString
                   withOptions:(IJKFFOptions *)options
           withSegmentResolver:(id<IJKMediaSegmentResolver>)segmentResolver;

- (void)prepareToPlay;
- (void)play;
- (void)pause;
- (void)stop;
- (BOOL)isPlaying;

- (void)setPauseInBackground:(BOOL)pause;

@end

#define IJK_FF_IO_TYPE_READ (1)
void IJKFFIOStatDebugCallback(const char *url, int type, int bytes);
void IJKFFIOStatRegister(void (*cb)(const char *url, int type, int bytes));

void IJKFFIOStatCompleteDebugCallback(const char *url,
                                      int64_t read_bytes, int64_t total_size,
                                      int64_t elpased_time, int64_t total_duration);
void IJKFFIOStatCompleteRegister(void (*cb)(const char *url,
                                            int64_t read_bytes, int64_t total_size,
                                            int64_t elpased_time, int64_t total_duration));
