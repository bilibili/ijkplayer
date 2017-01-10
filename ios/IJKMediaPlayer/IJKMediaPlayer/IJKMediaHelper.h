//
//  IJKMediaHelper.h
//  IJKMediaPlayer
//
//  Created by lsc on 09/01/2017.
//  Copyright © 2017 bilibili. All rights reserved.
//

#import <UIKit/UIKit.h>

typedef void (^CreateScreenshotCompletionHandler) (NSString *screenshotPath);

@interface IJKMediaHelper : NSObject

+ (void)createScreenshotOfVideoAtPath:(NSString*)path atTime:(NSTimeInterval)time size:(CGSize)size completion:(CreateScreenshotCompletionHandler)completion;
+ (NSTimeInterval)durationOfVideoAtPath:(NSString*)path;

@end
