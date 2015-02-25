//
//  IJKKVOController.h
//  IJKMediaPlayer
//
//  Created by Zhang Rui on 14-9-29.
//  Copyright (c) 2014年 bilibili. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface IJKKVOController : NSObject

- (id)initWithTarget:(NSObject *)target;

- (void)safelyAddObserver:(NSObject *)observer
               forKeyPath:(NSString *)keyPath
                  options:(NSKeyValueObservingOptions)options
                  context:(void *)context;
- (void)safelyRemoveObserver:(NSObject *)observer
                  forKeyPath:(NSString *)keyPath;

- (void)safelyRemoveAllObservers;

@end
