/*
 * Copyright (C) 2015 Gdier
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
