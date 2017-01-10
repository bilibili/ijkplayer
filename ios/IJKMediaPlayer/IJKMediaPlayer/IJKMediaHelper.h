//
//  IJKMediaHelper.h
//  IJKMediaPlayer
//
//  Created by lsc on 09/01/2017.
//  Copyright © 2017 bilibili. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface IJKMediaHelper : NSObject

+ (UIImage *)thumbnailOfVideoAtPath:(NSString*)path atTime:(NSTimeInterval)time;
+ (NSTimeInterval)durationOfVideoAtPath:(NSString*)path;

@end
