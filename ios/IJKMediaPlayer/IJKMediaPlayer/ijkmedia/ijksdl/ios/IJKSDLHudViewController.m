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
    CGRect _rect;
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

- (void)setRect:(CGRect) rect {
    _rect = rect;
}

- (void)viewWillLayoutSubviews {
    [super viewWillLayoutSubviews];

    CGRect selfFrame = _rect;
    CGRect newFrame  = selfFrame;

    newFrame.size.width   = selfFrame.size.width * 1 / 3;
    newFrame.origin.x     = selfFrame.size.width * 2 / 3;

    newFrame.size.height  = selfFrame.size.height * 8 / 8;
    newFrame.origin.y    += selfFrame.size.height * 0 / 8;

    self.tableView.frame = newFrame;
}

#pragma mark UITableViewDelegate

- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath
{
    return 16.f;
}

@end
