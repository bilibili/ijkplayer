//
//  IJKFFMoviePlayerController+OptionConvenience.h
//  IJKMediaPlayer
//
//  Created by Zhang Rui on 15/6/4.
//  Copyright (c) 2015年 bilibili. All rights reserved.
//

#import "IJKFFMoviePlayerController.h"

@interface IJKFFMoviePlayerController (OptionConvenience)

- (void)setFormatOptionValue:       (NSString *)value forKey:(NSString *)key;
- (void)setCodecOptionValue:        (NSString *)value forKey:(NSString *)key;
- (void)setSwsOptionValue:          (NSString *)value forKey:(NSString *)key;
- (void)setPlayerOptionValue:       (NSString *)value forKey:(NSString *)key;

- (void)setFormatOptionIntValue:    (NSInteger)value forKey:(NSString *)key;
- (void)setCodecOptionIntValue:     (NSInteger)value forKey:(NSString *)key;
- (void)setSwsOptionIntValue:       (NSInteger)value forKey:(NSString *)key;
- (void)setPlayerOptionIntValue:    (NSInteger)value forKey:(NSString *)key;

- (void)setMaxBufferSize:(int)maxBufferSize; // -1 for default size

@end
