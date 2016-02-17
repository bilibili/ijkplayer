//
//  NSString+IJKMedia.m
//  IJKMediaPlayer
//
//  Created by Zhang Rui on 15/12/15.
//  Copyright © 2015年 bilibili. All rights reserved.
//

#import "NSString+IJKMedia.h"

@implementation NSString (IJKMedia)

- (BOOL) ijk_isIpv4
{
    NSString *regexp =
    @"^(25[0-5]|2[0-4][0-9]|[0-1]{1}[0-9]{2}|[1-9]{1}[0-9]{1}|[1-9])\\.(25[0-5]|2[0-4][0-9]|[0-1]{1}[0-9]{2}|[1-9]{1}[0-9]{1}|[1-9]|0)\\.(25[0-5]|2[0-4][0-9]|[0-1]{1}[0-9]{2}|[1-9]{1}[0-9]{1}|[1-9]|0)\\.(25[0-5]|2[0-4][0-9]|[0-1]{1}[0-9]{2}|[1-9]{1}[0-9]{1}|[0-9])$";

    NSRange range = [self rangeOfString:regexp options:NSRegularExpressionSearch];
    return range.location != NSNotFound;
}

@end
