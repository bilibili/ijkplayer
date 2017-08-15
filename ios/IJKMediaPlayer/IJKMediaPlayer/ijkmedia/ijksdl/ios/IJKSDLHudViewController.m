//
//  IJKSDLHudViewController.m
//  IJKMediaPlayer
//
//  Created by Zhang Rui on 15/12/14.
//  Copyright © 2015年 bilibili. All rights reserved.
//

#import "IJKSDLHudViewController.h"
#import "IJKSDLHudViewCell.h"

@interface HudViewCellData : NSObject
@property(nonatomic) NSString *key;
@property(nonatomic) NSString *value;
@end

@implementation HudViewCellData
@end

@interface IJKSDLHudViewController() <UITableViewDataSource, UITableViewDelegate>

@end

@implementation IJKSDLHudViewController
{
    NSMutableDictionary *_keyIndexes;
    NSMutableArray      *_hudDataArray;
}

- (id)init
{
    self = [super init];
    if (self) {
        _keyIndexes = [[NSMutableDictionary alloc] init];
        _hudDataArray = [[NSMutableArray alloc] init];

        self.tableView.backgroundColor = [[UIColor alloc] initWithRed:.5f green:.5f blue:.5f alpha:.5f];
        self.tableView.separatorStyle  = UITableViewCellSeparatorStyleNone;
    }
    return self;
}

- (void)setHudValue:(NSString *)value forKey:(NSString *)key
{
    HudViewCellData *data = nil;
    NSNumber *index = [_keyIndexes objectForKey:key];
    if (index == nil) {
        data = [[HudViewCellData alloc] init];
        data.key = key;
        [_keyIndexes setObject:[NSNumber numberWithUnsignedInteger:_hudDataArray.count]
                        forKey:key];
        [_hudDataArray addObject:data];
    } else {
        data = [_hudDataArray objectAtIndex:[index unsignedIntegerValue]];
    }

    data.value = value;
    [self.tableView reloadData];
}

#pragma mark UITableViewDataSource

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    assert(section == 0);
    return _hudDataArray.count;
}

- (UITableViewCell *)tableView:(UITableView *)tableView
         cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    assert(indexPath.section == 0);

    IJKSDLHudViewCell *cell = [tableView dequeueReusableCellWithIdentifier:@"hud"];
    if (cell == nil) {
        cell = [[IJKSDLHudViewCell alloc] init];
    }

    HudViewCellData *data = [_hudDataArray objectAtIndex:indexPath.item];

    [cell setHudValue:data.value forKey:data.key];

    return cell;
}

#pragma mark UITableViewDelegate

- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath
{
    return 16.f;
}

@end
