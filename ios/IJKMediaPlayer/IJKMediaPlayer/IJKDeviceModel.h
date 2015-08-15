/*
 * IJKDeviceModel.h
 *
 * Copyright (c) 2015 Zhang Rui <bbcallen@gmail.com>
 *
 * This file is part of ijkPlayer.
 *
 * ijkPlayer is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * ijkPlayer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with ijkPlayer; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#import <Foundation/Foundation.h>

#define kIJKDeviceRank_Baseline                         10
#define kIJKDeviceRank_AppleA4Class                     20   // Cortex-A8 class
#define kIJKDeviceRank_AppleA5Class                     30   // Cortex-A9 class
#define kIJKDeviceRank_AppleA5RAClass                   31   // Cortex-A9 class
#define kIJKDeviceRank_AppleA5XClass                    35   // Cortex-A9 class
#define kIJKDeviceRank_AppleA6Class                     40   // ARMv7s class
#define kIJKDeviceRank_AppleA6XClass                    41   // ARMv7s class
#define kIJKDeviceRank_AppleA7Class                     50   // ARM64 class
#define kIJKDeviceRank_AppleA8LowClass                  55   // ARM64 class
#define kIJKDeviceRank_AppleA8Class                     60   // ARM64 class
#define kIJKDeviceRank_AppleA8XClass                    61   // ARM64 class
#define kIJKDeviceRank_LatestUnknown                    90
#define kIJKDeviceRank_Simulator                        100

@interface IJKDeviceModel : NSObject

@property(nonatomic) NSString   *platform;
@property(nonatomic) NSString   *name;
@property(nonatomic) NSInteger  rank;

+ (instancetype)currentModel;

@end
