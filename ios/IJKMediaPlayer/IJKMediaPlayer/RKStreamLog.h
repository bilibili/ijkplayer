//
//  RKStreamLog.h
//  LFLiveKit
//
//  Created by Ken Sun on 2017/9/21.
//  Copyright © 2017年 admin. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface RKStreamLog : NSObject

// user id
@property (copy, nonatomic) NSString *uid;
// provider
@property (copy, nonatomic) NSString *pd;
// os type
@property (copy, nonatomic) NSString *os;
// os version
@property (copy, nonatomic) NSString *osv;
// phone model
@property (copy, nonatomic) NSString *mod;
// carrier
@property (copy, nonatomic) NSString *cr;
// net type,2g,3g,4g,wifi
@property (copy, nonatomic) NSString *nt;
// logitude
@property (assign, nonatomic) double lnt;
// latitude
@property (assign, nonatomic) double ltt;
// region
@property (copy, nonatomic) NSString *rg;
// app version
@property (copy, nonatomic) NSString *av17;
// url host
@property (copy, nonatomic) NSString *host;
// url
@property (copy, nonatomic) NSString *url;
// stream id
@property (copy, nonatomic) NSString *sid;
// init start time
@property (assign, nonatomic) NSTimeInterval initStartTime;


@property (copy, nonatomic) void(^logCallback)(NSDictionary *log);

+ (instancetype)logger;

- (void)logWithDict:(NSDictionary *)dic;

- (void)fetchInfo;

- (void)fetchHostStatus;

- (NSMutableDictionary *)basicLog;

@end


