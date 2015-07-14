//
//  WSPX.h
//  WSPX
//
//  Created by lincz on 13-1-17.
//  Copyright (c) 2013年 Chinanetcenter. All rights reserved.
//

#import <Foundation/Foundation.h>


@interface WSPX : NSObject

/*
** 开启服务
** 返回值指示了成功与否。
**
** 请在启动 app 后，首先调用此 API。在整个进程的生命周期中，该函数只调用一次。
*/
+ (BOOL)start;

/*
 ** 激活服务
 ** 返回值指示了成功与否。
 ** 
 ** 请在 app 进入前台后首先调用此 API。
 */
+ (BOOL)activate;


/*
 ** 关闭服务
 ** 
 ** 请在完全退出 app 时调用此 API。在整个进程的生命周期中，该函数只调用一次。
 */
+ (void)stop;


/*
 ** 设置是否走代理
 **
 ** 这是一个全局开关
 */
+ (void)setViaProxy:(BOOL)value;


/*
 ** 创建一个tcp socket
 ** 成功返回sockfd
 ** 错误返回－1
 **
 */
+ (int)tcp_connect:(const char*)ipaddr port:(int)port;

/*
 ** 视频加速接口
 ** 返回加速后的url
 **
 */
+ (NSString *)getProxifiedUrl:(NSString *)originalUrl;



@end



