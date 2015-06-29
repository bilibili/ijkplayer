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
 * This formatter can be used to chain different formatters together.
 * The log message will processed in the order of the formatters added.
 **/

@interface DDMultiFormatter : NSObject <DDLogFormatter>

/**
 *  Array of chained formatters
 */
@property (readonly) NSArray *formatters;

- (void)addFormatter:(id<DDLogFormatter>)formatter;
- (void)removeFormatter:(id<DDLogFormatter>)formatter;
- (void)removeAllFormatters;
- (BOOL)isFormattingWithFormatter:(id<DDLogFormatter>)formatter;

@end
