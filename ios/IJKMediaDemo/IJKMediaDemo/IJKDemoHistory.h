//
//  IJKDemoHistory.h
//  IJKMediaDemo
//
//  Created by Gdier on 5/26/15.
//  Copyright (c) 2015 bilibili. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface IJKDemoHistoryItem : NSObject <NSCoding>

@property(nonatomic,strong) NSString *title;
@property(nonatomic,strong) NSURL *url;

@end

@interface IJKDemoHistory : NSObject

+ (instancetype)instance;

@property(nonatomic,strong,readonly) NSArray *list;

- (void)removeAtIndex:(NSUInteger)index;
- (void)add:(IJKDemoHistoryItem *)item;

@end
