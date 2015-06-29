#import <Foundation/Foundation.h>
#import "DDLog.h"

/**
 * Welcome to Cocoa Lumberjack!
 * 
 * The project page has a wealth of documentation if you have any questions.
 * https://github.com/CocoaLumberjack/CocoaLumberjack
 * 
 * If you're new to the project you may wish to read the "Getting Started" page.
 * https://github.com/CocoaLumberjack/CocoaLumberjack/wiki/GettingStarted
 * 
 * 
 * This class provides a log formatter that filters log statements from a logging context not on the whitelist.
 * 
 * A log formatter can be added to any logger to format and/or filter its output.
 * You can learn more about log formatters here:
 * https://github.com/CocoaLumberjack/CocoaLumberjack/wiki/CustomFormatters
 * 
 * You can learn more about logging context's here:
 * https://github.com/CocoaLumberjack/CocoaLumberjack/wiki/CustomContext
 *
 * But here's a quick overview / refresher:
 * 
 * Every log statement has a logging context.
 * These come from the underlying logging macros defined in DDLog.h.
 * The default logging context is zero.
 * You can define multiple logging context's for use in your application.
 * For example, logically separate parts of your app each have a different logging context.
 * Also 3rd party frameworks that make use of Lumberjack generally use their own dedicated logging context.
**/
@interface DDContextWhitelistFilterLogFormatter : NSObject <DDLogFormatter>

- (id)init;

- (void)addToWhitelist:(int)loggingContext;
- (void)removeFromWhitelist:(int)loggingContext;

- (NSArray *)whitelist;

- (BOOL)isOnWhitelist:(int)loggingContext;

@end

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma mark -
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * This class provides a log formatter that filters log statements from a logging context on the blacklist.
**/
@interface DDContextBlacklistFilterLogFormatter : NSObject <DDLogFormatter>

- (id)init;

- (void)addToBlacklist:(int)loggingContext;
- (void)removeFromBlacklist:(int)loggingContext;

- (NSArray *)blacklist;

- (BOOL)isOnBlacklist:(int)loggingContext;

@end
