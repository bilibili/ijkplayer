//
//  IJKFFPlayerDef.h
//  IJKMediaPlayer
//
//  Created by ZhangRui on 13-9-25.
//  Copyright (c) 2013年 bilibili. All rights reserved.
//

#import <Foundation/Foundation.h>
#include "ijkplayer/ios/ijkplayer_ios.h"

@interface IJKFFPlayerMessage : NSObject {
@public
    AVMessage _msg;
}
@end



struct IJKSize {
    NSInteger width;
    NSInteger height;
};
typedef struct IJKSize IJKSize;

struct IJKIntPair {
    NSInteger arg1;
    NSInteger arg2;
};
typedef struct IJKIntPair IJKIntPair;



CG_INLINE IJKSize
IJKSizeMake(NSInteger width, NSInteger height)
{
    IJKSize size;
    size.width = width;
    size.height = height;
    return size;
}

CG_INLINE IJKIntPair
IJKIntPairMake(NSInteger arg1, NSInteger arg2)
{
    IJKIntPair pair;
    pair.arg1 = arg1;
    pair.arg2 = arg2;
    return pair;
}