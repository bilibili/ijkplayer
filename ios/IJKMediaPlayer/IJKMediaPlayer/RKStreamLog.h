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
// log type
@property (copy, nonatomic) NSString *lt;
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
// longitude
@property (copy, nonatomic) NSString *lnt;
// latitude
@property (copy, nonatomic) NSString *ltt;
// region
@property (copy, nonatomic) NSString *rg;
// app version
@property (copy, nonatomic) NSString *av17;
// url host
@property (copy, nonatomic) NSString *host;
// scheme, protocal type
@property (copy, nonatomic) NSString *pt;
// url
@property (copy, nonatomic) NSString *url;
// stream id
@property (copy, nonatomic) NSString *sid;
// ping round trip interval
@property (copy, nonatomic) NSString *pingRtt;
// ping packet loss
@property (copy, nonatomic) NSString *pingloss;


@property (copy, nonatomic) void(^logCallback)(NSDictionary *log);

+ (instancetype)logger;

- (void)logWithDict:(NSDictionary *)dic;

- (void)fetchInfo;

- (void)fetchHostStatus;

@end
