/*
 * IJKDeviceModel.m
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

#import "IJKDeviceModel.h"

#include <sys/utsname.h>

@implementation IJKDeviceModel

- (instancetype)initWithPlatform:(NSString *)platform
                        withName:(NSString *)name
                        withRank:(NSInteger)rank
{
    self = [super init];
    if (self) {
        _platform = platform;
        _name     = name;
        _rank     = rank;
    }
    return self;
}

inline static void IJKDeviceRegister(NSMutableDictionary *dict,
                                    NSString *platform,
                                    NSString *name,
                                    NSInteger rank)
{
    IJKDeviceModel *model = [[IJKDeviceModel alloc] initWithPlatform:platform
                                                            withName:name
                                                            withRank:rank];
    [dict setObject:model forKey:platform];
}

+ (instancetype)modelWithName:(NSString *)name
{
    static IJKDeviceModel  *sLatestModel = nil;
    static NSDictionary    *sModelDictionary = nil;
    static dispatch_once_t  sOnceToken = 0;
    dispatch_once(&sOnceToken, ^{
        NSMutableDictionary *dict = [[NSMutableDictionary alloc] init];

        // http://en.wikipedia.org/wiki/List_of_iOS_devices
        // Simulator
        IJKDeviceRegister(dict, @"i386",        @"Simulator",       kIJKDeviceRank_Simulator);
        IJKDeviceRegister(dict, @"x86_64",      @"Simulator 64",    kIJKDeviceRank_Simulator);

        // iPhone
        IJKDeviceRegister(dict, @"iPhone1",     @"iPhone",          kIJKDeviceRank_Baseline);
        IJKDeviceRegister(dict, @"iPhone1,1",   @"iPhone",          kIJKDeviceRank_Baseline);
        IJKDeviceRegister(dict, @"iPhone1,2",   @"iPhone 3G",       kIJKDeviceRank_Baseline);
        IJKDeviceRegister(dict, @"iPhone2",     @"iPhone 3GS",      kIJKDeviceRank_Baseline);
        IJKDeviceRegister(dict, @"iPhone2,1",   @"iPhone 3GS",      kIJKDeviceRank_Baseline);

        IJKDeviceRegister(dict, @"iPhone3",     @"iPhone 4",        kIJKDeviceRank_AppleA4Class);
        IJKDeviceRegister(dict, @"iPhone3,1",   @"iPhone 4",        kIJKDeviceRank_AppleA4Class);
        IJKDeviceRegister(dict, @"iPhone3,2",   @"iPhone 4",        kIJKDeviceRank_AppleA4Class);
        IJKDeviceRegister(dict, @"iPhone3,3",   @"iPhone 4",        kIJKDeviceRank_AppleA4Class);

        IJKDeviceRegister(dict, @"iPhone4",     @"iPhone 4s",       kIJKDeviceRank_AppleA5Class);
        IJKDeviceRegister(dict, @"iPhone4,1",   @"iPhone 4s",       kIJKDeviceRank_AppleA5Class);

        IJKDeviceRegister(dict, @"iPhone5",     @"iPhone 5/5c",     kIJKDeviceRank_AppleA6Class);
        IJKDeviceRegister(dict, @"iPhone5,1",   @"iPhone 5",        kIJKDeviceRank_AppleA6Class);
        IJKDeviceRegister(dict, @"iPhone5,2",   @"iPhone 5",        kIJKDeviceRank_AppleA6Class);
        IJKDeviceRegister(dict, @"iPhone5,3",   @"iPhone 5c",       kIJKDeviceRank_AppleA6Class);
        IJKDeviceRegister(dict, @"iPhone5,4",   @"iPhone 5c",       kIJKDeviceRank_AppleA6Class);

        IJKDeviceRegister(dict, @"iPhone6",     @"iPhone 5s",       kIJKDeviceRank_AppleA7Class);
        IJKDeviceRegister(dict, @"iPhone6,1",   @"iPhone 5s",       kIJKDeviceRank_AppleA7Class);
        IJKDeviceRegister(dict, @"iPhone6,2",   @"iPhone 5s",       kIJKDeviceRank_AppleA7Class);

        IJKDeviceRegister(dict, @"iPhone7",     @"iPhone 6",        kIJKDeviceRank_AppleA8Class);
        IJKDeviceRegister(dict, @"iPhone7,1",   @"iPhone 6 Plus",   kIJKDeviceRank_AppleA8Class);
        IJKDeviceRegister(dict, @"iPhone7,2",   @"iPhone 6",        kIJKDeviceRank_AppleA8Class);

        // iPod Touch
        IJKDeviceRegister(dict, @"iPod1",       @"iPod Touch",      kIJKDeviceRank_Baseline);
        IJKDeviceRegister(dict, @"iPod1,1",     @"iPod Touch",      kIJKDeviceRank_Baseline);
        IJKDeviceRegister(dict, @"iPod2",       @"iPod Touch 2G",   kIJKDeviceRank_Baseline);
        IJKDeviceRegister(dict, @"iPod2,1",     @"iPod Touch 2G",   kIJKDeviceRank_Baseline);
        IJKDeviceRegister(dict, @"iPod3",       @"iPod Touch 3G",   kIJKDeviceRank_Baseline);
        IJKDeviceRegister(dict, @"iPod3,1",     @"iPod Touch 3G",   kIJKDeviceRank_Baseline);

        IJKDeviceRegister(dict, @"iPod4",       @"iPod Touch 4G",   kIJKDeviceRank_AppleA4Class);
        IJKDeviceRegister(dict, @"iPod4,1",     @"iPod Touch 4G",   kIJKDeviceRank_AppleA4Class);

        IJKDeviceRegister(dict, @"iPod5",       @"iPod Touch 5G",   kIJKDeviceRank_AppleA5Class);
        IJKDeviceRegister(dict, @"iPod5,1",     @"iPod Touch 5G",   kIJKDeviceRank_AppleA5Class);

        IJKDeviceRegister(dict, @"iPod7",       @"iPod Touch 6G",   kIJKDeviceRank_AppleA8LowClass);
        IJKDeviceRegister(dict, @"iPod7,1",     @"iPod Touch 6G",   kIJKDeviceRank_AppleA8LowClass);

        // iPad / iPad mini
        IJKDeviceRegister(dict, @"iPad1",       @"iPad",            kIJKDeviceRank_AppleA4Class);
        IJKDeviceRegister(dict, @"iPad1,1",     @"iPad",            kIJKDeviceRank_AppleA4Class);

        IJKDeviceRegister(dict, @"iPad2",       @"iPad 2/mini",     kIJKDeviceRank_AppleA5Class);
        IJKDeviceRegister(dict, @"iPad2,1",     @"iPad 2",          kIJKDeviceRank_AppleA5Class);
        IJKDeviceRegister(dict, @"iPad2,2",     @"iPad 2",          kIJKDeviceRank_AppleA5Class);
        IJKDeviceRegister(dict, @"iPad2,3",     @"iPad 2",          kIJKDeviceRank_AppleA5Class);

        IJKDeviceRegister(dict, @"iPad2,4",     @"iPad 2",          kIJKDeviceRank_AppleA5RAClass);
        IJKDeviceRegister(dict, @"iPad2,5",     @"iPad mini",       kIJKDeviceRank_AppleA5RAClass);
        IJKDeviceRegister(dict, @"iPad2,6",     @"iPad mini",       kIJKDeviceRank_AppleA5RAClass);
        IJKDeviceRegister(dict, @"iPad2,7",     @"iPad mini",       kIJKDeviceRank_AppleA5RAClass);

        IJKDeviceRegister(dict, @"iPad3,1",     @"iPad 3G",         kIJKDeviceRank_AppleA5XClass);
        IJKDeviceRegister(dict, @"iPad3,2",     @"iPad 3G",         kIJKDeviceRank_AppleA5XClass);
        IJKDeviceRegister(dict, @"iPad3,3",     @"iPad 3G",         kIJKDeviceRank_AppleA5XClass);

        IJKDeviceRegister(dict, @"iPad3,4",     @"iPad 4G",         kIJKDeviceRank_AppleA6XClass);
        IJKDeviceRegister(dict, @"iPad3,5",     @"iPad 4G",         kIJKDeviceRank_AppleA6XClass);
        IJKDeviceRegister(dict, @"iPad3,6",     @"iPad 4G",         kIJKDeviceRank_AppleA6XClass);

        IJKDeviceRegister(dict, @"iPad4",       @"iPad Air/Mini 2G/3G",   kIJKDeviceRank_AppleA7Class);
        IJKDeviceRegister(dict, @"iPad4,1",     @"iPad Air",        kIJKDeviceRank_AppleA7Class);
        IJKDeviceRegister(dict, @"iPad4,2",     @"iPad Air",        kIJKDeviceRank_AppleA7Class);
        IJKDeviceRegister(dict, @"iPad4,3",     @"iPad Air",        kIJKDeviceRank_AppleA7Class);
        IJKDeviceRegister(dict, @"iPad4,4",     @"iPad mini 2G",    kIJKDeviceRank_AppleA7Class);
        IJKDeviceRegister(dict, @"iPad4,5",     @"iPad mini 2G",    kIJKDeviceRank_AppleA7Class);
        IJKDeviceRegister(dict, @"iPad4,6",     @"iPad mini 2G",    kIJKDeviceRank_AppleA7Class);

        IJKDeviceRegister(dict, @"iPad4,7",     @"iPad mini 3",     kIJKDeviceRank_AppleA7Class);
        IJKDeviceRegister(dict, @"iPad4,8",     @"iPad mini 3",     kIJKDeviceRank_AppleA7Class);
        IJKDeviceRegister(dict, @"iPad4,9",     @"iPad mini 3",     kIJKDeviceRank_AppleA7Class);

        IJKDeviceRegister(dict, @"iPad5",       @"iPad Air 2",      kIJKDeviceRank_AppleA8XClass);
        IJKDeviceRegister(dict, @"iPad5,3",     @"iPad Air 2",      kIJKDeviceRank_AppleA8XClass);
        IJKDeviceRegister(dict, @"iPad5,4",     @"iPad Air 2",      kIJKDeviceRank_AppleA8XClass);

        sModelDictionary = dict;

        sLatestModel = [[IJKDeviceModel alloc] initWithPlatform:name
                                                       withName:name
                                                       withRank:kIJKDeviceRank_LatestUnknown];
    });

    IJKDeviceModel *model = [sModelDictionary objectForKey:name];
    if (model == nil) {
        if (model == nil) {
            model = sLatestModel;
        }

        NSArray *components = [name componentsSeparatedByString:@","];
        if (components != nil && components.count > 0) {
            NSString *majorName = components[0];
            if (majorName != nil) {
                majorName = [majorName stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
                model = [sModelDictionary objectForKey:majorName];
            }
        }
    }

    return model;
}

+ (NSString *)currentModelName
{
    struct utsname systemInfo;
    uname(&systemInfo);

    return [NSString stringWithUTF8String:systemInfo.machine];
}

+ (instancetype)currentModel
{
    return [IJKDeviceModel modelWithName:[IJKDeviceModel currentModelName]];
}

@end
