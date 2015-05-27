//
//  IJKDemoHistory.m
//  IJKMediaDemo
//
//  Created by Gdier on 5/26/15.
//  Copyright (c) 2015 bilibili. All rights reserved.
//

#import "IJKDemoHistory.h"

@implementation IJKDemoHistoryItem

- (void)encodeWithCoder:(NSCoder *)aCoder {
    [aCoder encodeObject:self.url forKey:@"url"];
    [aCoder encodeObject:self.title forKey:@"title"];
}

- (instancetype)initWithCoder:(NSCoder *)coder {
    self = [super init];
    if (self) {
        self.title = [coder decodeObjectForKey:@"title"];
        self.url = [coder decodeObjectForKey:@"url"];
    }
    return self;
}

@end

@interface IJKDemoHistory ()

@end

@implementation IJKDemoHistory {
    NSMutableArray *_list;
}

+ (instancetype)instance {
    static IJKDemoHistory *s_obj = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        s_obj = [[IJKDemoHistory alloc] init];
    });
    
    return s_obj;
}

- (instancetype)init {
    self = [super init];
    if (self) {
        _list = [NSKeyedUnarchiver unarchiveObjectWithFile:[self dbfilePath]];
        if (nil == _list)
            _list = [NSMutableArray array];
    }
    return self;
}

- (NSString *)dbfilePath {
    NSString *libraryPath = [NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSAllDomainsMask, YES) firstObject];
    libraryPath = [libraryPath stringByAppendingPathComponent:@"ijkhistory.plist"];
    
    return libraryPath;
}

- (NSArray *)list {
    return _list;
}

- (void)removeAtIndex:(NSUInteger)index {
    [_list removeObjectAtIndex:index];
    
    [NSKeyedArchiver archiveRootObject:_list toFile:[self dbfilePath]];
}

- (void)add:(IJKDemoHistoryItem *)item {
    __block NSUInteger findIdx = NSNotFound;
    
    [_list enumerateObjectsUsingBlock:^(IJKDemoHistoryItem *enumItem, NSUInteger idx, BOOL *stop) {
        if ([enumItem.url isEqual:item.url]) {
            findIdx = idx;
            *stop = YES;
        }
    }];
    
    if (NSNotFound != findIdx) {
        [_list removeObjectAtIndex:findIdx];
    }
    
    [_list insertObject:item atIndex:0];
    
    [NSKeyedArchiver archiveRootObject:_list toFile:[self dbfilePath]];
}

@end
