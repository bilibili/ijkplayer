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


@interface IJKFFPlayerMessagePool : NSObject

- (IJKFFPlayerMessagePool *)init;
- (IJKFFPlayerMessage *) obtain;
- (void) recycle:(IJKFFPlayerMessage *)msg;

@end
