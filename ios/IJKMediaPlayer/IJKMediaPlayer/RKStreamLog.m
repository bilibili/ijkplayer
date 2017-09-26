//
//  RKStreamLog.m
//  LFLiveKit
//
//  Created by Ken Sun on 2017/9/21.
//  Copyright © 2017年 admin. All rights reserved.
//

#import "RKStreamLog.h"
#import "STDPingServices.h"
#import <sys/utsname.h>
#import <CoreTelephony/CTTelephonyNetworkInfo.h>
#import <CoreTelephony/CTCarrier.h>
#import <UIKit/UIKit.h>
#import <SystemConfiguration/SystemConfiguration.h>

@implementation RKStreamLog {
    STDPingServices *_ping;
}

+ (instancetype)logger {
    static RKStreamLog *logger = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        logger = [[RKStreamLog alloc] init];
    });
    return logger;
}

- (NSMutableDictionary *)basicLog {
    NSMutableDictionary *dic = [NSMutableDictionary new];
    dic[@"tm"] = @([NSDate date].timeIntervalSince1970);
    if (self.uid) dic[@"uid"] = self.uid;
    if (self.pd) dic[@"pd"] = self.pd;
    if (self.lt) dic[@"lt"] = self.lt;
    if (self.os) dic[@"os"] = self.os;
    if (self.osv) dic[@"osv"] = self.osv;
    if (self.mod) dic[@"mod"] = self.mod;
    if (self.cr) dic[@"cr"] = self.cr;
    if (self.nt) dic[@"nt"] = self.nt;
    if (self.lnt) dic[@"lnt"] = self.lnt;
    if (self.ltt) dic[@"ltt"] = self.ltt;
    if (self.rg) dic[@"rg"] = self.rg;
    if (self.av17) dic[@"av17"] = self.av17;
    if (self.host) dic[@"host"] = self.host;
    if (self.pt) dic[@"pt"] = self.pt;
    //if (self.url) dic[@"url"] = self.url;
    if (self.sid) dic[@"sid"] = self.sid;
    return dic;
}

- (void)logWithDict:(NSDictionary *)dic {
    NSMutableDictionary *logInfo = [self basicLog];
    [logInfo addEntriesFromDictionary:dic];
    if (self.logCallback) {
        self.logCallback(logInfo);
    }
}

- (void)fetchInfo {
    [self fetchCarrier];
    [self fetchDevice];
}

- (void)fetchCarrier {
    // 获取本机运营商名称
    CTTelephonyNetworkInfo *info = [[CTTelephonyNetworkInfo alloc] init];
    CTCarrier *carrier = [info subscriberCellularProvider];
    
    // 当前手机所属运营商名称
    NSString *mobile;
    
    // 先判断有没有SIM卡，如果没有则不获取本机运营商
    if (!carrier.isoCountryCode) {
        NSLog(@"没有SIM卡");
        mobile = @"无运营商";
    } else {
        mobile = [carrier carrierName];
    }
    self.cr = mobile;
    
    SCNetworkReachabilityRef reachability = SCNetworkReachabilityCreateWithName(NULL, "8.8.8.8");
    SCNetworkReachabilityFlags flags;
    BOOL success = SCNetworkReachabilityGetFlags(reachability, &flags);
    CFRelease(reachability);
    if (!success) {
        return;
    }
    if ((flags & kSCNetworkReachabilityFlagsIsWWAN) != 0) {
        NSString * carrierType = info.currentRadioAccessTechnology;
        if ([carrierType isEqualToString:CTRadioAccessTechnologyGPRS]) {
            self.nt = @"2G";
        } else if ([carrierType isEqualToString:CTRadioAccessTechnologyEdge]) {
            self.nt = @"2G";
        } else if ([carrierType isEqualToString:CTRadioAccessTechnologyCDMA1x]) {
            self.nt = @"2G";
        } else if ([carrierType isEqualToString:CTRadioAccessTechnologyWCDMA]) {
            self.nt = @"3G";
        } else if ([carrierType isEqualToString:CTRadioAccessTechnologyHSDPA]) {
            self.nt = @"3G";
        } else if ([carrierType isEqualToString:CTRadioAccessTechnologyHSUPA]) {
            self.nt = @"3G";
        } else if ([carrierType isEqualToString:CTRadioAccessTechnologyCDMAEVDORev0]) {
            self.nt = @"3G";
        } else if ([carrierType isEqualToString:CTRadioAccessTechnologyCDMAEVDORevA]) {
            self.nt = @"3G";
        } else if ([carrierType isEqualToString:CTRadioAccessTechnologyCDMAEVDORevB]) {
            self.nt = @"3G";
        } else if ([carrierType isEqualToString:CTRadioAccessTechnologyeHRPD]) {
            self.nt = @"3G";
        } else if ([carrierType isEqualToString:CTRadioAccessTechnologyLTE]) {
            self.nt = @"4G";
        }
    } else {
        self.nt = @"WiFi";
    }
}

- (void)fetchDevice {
    self.os = [[UIDevice currentDevice] systemName]; // "iPhone OS" //系统名称
    self.osv = [[UIDevice currentDevice] systemVersion]; // "2.2.1” //系统版本号

    struct utsname systemInfo;
    uname(&systemInfo);
    self.mod = [NSString stringWithCString:systemInfo.machine encoding:NSASCIIStringEncoding];
}

- (void)fetchHostStatus {
    __weak typeof(self) wSelf = self;
    _ping = [STDPingServices startPingAddress:self.host times:15 handler:^(STDPingItem *pingItem, NSArray *pingItems) {
        if (pingItem.status != STDPingStatusFinished) {
            NSLog(@"%@", pingItem.description);
        } else {
            wSelf.pingloss = [NSString stringWithFormat:@"%f", _ping.lossPercentage];
            wSelf.pingRtt = [NSString stringWithFormat:@"%li",_ping.averageRetryTime];
            [wSelf logWithDict:@{@"lt": @"pv",
                                 @"prtt": wSelf.pingRtt,
                                 @"plss": wSelf.pingloss
                                 }];
        }
    }];
}

@end
