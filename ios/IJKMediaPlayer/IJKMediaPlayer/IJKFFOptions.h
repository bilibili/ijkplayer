//
//  IJKFFOptions.h
//  IJKMediaPlayer
//
//  Created by ZhangRui on 13-10-17.
//  Copyright (c) 2013年 bilibili. All rights reserved.
//

#import <Foundation/Foundation.h>

typedef enum IJKFFOptionCategory {
    kIJKFFOptionCategoryFormat = 1,
    kIJKFFOptionCategoryCodec  = 2,
    kIJKFFOptionCategorySws    = 3,
    kIJKFFOptionCategoryPlayer = 4,
} IJKFFOptionCategory;

typedef enum IJKAVDiscard {
    /* We leave some space between them for extensions (drop some
     * keyframes for intra-only or drop just some bidir frames). */
    IJK_AVDISCARD_NONE    =-16, ///< discard nothing
    IJK_AVDISCARD_DEFAULT =  0, ///< discard useless packets like 0 size packets in avi
    IJK_AVDISCARD_NONREF  =  8, ///< discard all non reference
    IJK_AVDISCARD_BIDIR   = 16, ///< discard all bidirectional frames
    IJK_AVDISCARD_NONKEY  = 32, ///< discard all frames except keyframes
    IJK_AVDISCARD_ALL     = 48, ///< discard all
} IJKAVDiscard;

typedef struct IjkMediaPlayer IjkMediaPlayer;

@interface IJKFFOptions : NSObject

+(IJKFFOptions *)optionsByDefault;

-(void)applyTo:(IjkMediaPlayer *)mediaPlayer;

// use setOptionValue instead
@property(nonatomic) IJKAVDiscard skipLoopFilter;
@property(nonatomic) IJKAVDiscard skipFrame;

@property(nonatomic) int    frameBufferCount;
@property(nonatomic) int    maxFps;
@property(nonatomic) int    frameDrop;
@property(nonatomic) BOOL   pauseInBackground;
@property(nonatomic) BOOL   videoToolboxEnabled;
@property(nonatomic) int    frameMaxWidth;
@property(nonatomic) BOOL   autoReconnect;

@property(nonatomic, strong) NSString* userAgent;

@property(nonatomic) int64_t timeout; ///< read/write timeout, -1 for infinite, in microseconds

- (void)setOptionValue:(NSString *)value
                forKey:(NSString *)key
            ofCategory:(IJKFFOptionCategory)category;

- (void)setOptionIntValue:(NSInteger)value
                   forKey:(NSString *)key
               ofCategory:(IJKFFOptionCategory)category;

@end
