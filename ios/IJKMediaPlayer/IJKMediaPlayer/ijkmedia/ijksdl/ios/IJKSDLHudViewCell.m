//
//  IJKSDLHudViewCell.m
//  IJKMediaPlayer
//
//  Created by Zhang Rui on 15/12/14.
//  Copyright © 2015年 bilibili. All rights reserved.
//

#import "IJKSDLHudViewCell.h"

#define COLUMN_COUNT    2
#define CELL_MARGIN     8

@interface IJKSDLHudViewCell()

@end

@implementation IJKSDLHudViewCell
{
    UILabel *_column[COLUMN_COUNT];
}

- (id)init
{
    self = [super init];
    if (self) {
        self.backgroundColor = [UIColor clearColor];
        self.contentView.backgroundColor = [UIColor clearColor];

        for (int i = 0; i < COLUMN_COUNT; ++i) {
            _column[i] = [[UILabel alloc] init];
            _column[i].textColor = [UIColor whiteColor];
            _column[i].font = [UIFont fontWithName:@"Menlo" size:9];
            _column[i].adjustsFontSizeToFitWidth = YES;
            _column[i].numberOfLines = 1;
            _column[i].minimumScaleFactor = 0.5;
            [self.contentView addSubview:_column[i]];
        }
    }
    return self;
}

- (void)setHudValue:(NSString *)value forKey:(NSString *)key
{
    _column[0].text = key;
    _column[1].text = value;
}

- (void)layoutSubviews
{
    [super layoutSubviews];

    CGRect parentFrame = self.contentView.frame;
    CGRect newFrame    = parentFrame;
    CGFloat nextX      = CELL_MARGIN;

    newFrame.origin.x   = nextX;
    newFrame.size.width = parentFrame.size.width * 0.3;
    _column[0].frame    = newFrame;
    nextX               = newFrame.origin.x + newFrame.size.width + CELL_MARGIN;

    newFrame.origin.x   = nextX;
    newFrame.size.width = parentFrame.size.width - nextX - CELL_MARGIN;
    _column[1].frame = newFrame;
}

@end
