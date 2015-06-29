#import <Foundation/Foundation.h>
#if TARGET_OS_IPHONE
#import <UIKit/UIColor.h>
#else
#import <AppKit/NSColor.h>
#endif

#import "DDLog.h"

#define LOG_CONTEXT_ALL INT_MAX

/**
 * Welcome to Cocoa Lumberjack!
 * 
 * The project page has a wealth of documentation if you have any questions.
 * https://github.com/CocoaLumberjack/CocoaLumberjack
 * 
 * If you're new to the project you may wish to read the "Getting Started" wiki.
 * https://github.com/CocoaLumberjack/CocoaLumberjack/wiki/GettingStarted
 * 
 * 
 * This class provides a logger for Terminal output or Xcode console output,
 * depending on where you are running your code.
 * 
 * As described in the "Getting Started" page,
 * the traditional NSLog() function directs it's output to two places:
 * 
 * - Apple System Log (so it shows up in Console.app)
 * - StdErr (if stderr is a TTY, so log statements show up in Xcode console)
 * 
 * To duplicate NSLog() functionality you can simply add this logger and an asl logger.
 * However, if you instead choose to use file logging (for faster performance),
 * you may choose to use only a file logger and a tty logger.
**/

@interface DDTTYLogger : DDAbstractLogger <DDLogger>
{
    NSCalendar *calendar;
    NSUInteger calendarUnitFlags;
    
    NSString *appName;
    char *app;
    size_t appLen;
    
    NSString *processID;
    char *pid;
    size_t pidLen;
    
    BOOL colorsEnabled;
    NSMutableArray *colorProfilesArray;
    NSMutableDictionary *colorProfilesDict;
}

+ (instancetype)sharedInstance;

/* Inherited from the DDLogger protocol:
 * 
 * Formatters may optionally be added to any logger.
 * 
 * If no formatter is set, the logger simply logs the message as it is given in logMessage,
 * or it may use its own built in formatting style.
 * 
 * More information about formatters can be found here:
 * https://github.com/CocoaLumberjack/CocoaLumberjack/wiki/CustomFormatters
 * 
 * The actual implementation of these methods is inherited from DDAbstractLogger.

- (id <DDLogFormatter>)logFormatter;
- (void)setLogFormatter:(id <DDLogFormatter>)formatter;
 
*/

/**
 * Want to use different colors for different log levels?
 * Enable this property.
 * 
 * If you run the application via the Terminal (not Xcode),
 * the logger will map colors to xterm-256color or xterm-color (if available).
 * 
 * Xcode does NOT natively support colors in the Xcode debugging console.
 * You'll need to install the XcodeColors plugin to see colors in the Xcode console.
 * https://github.com/robbiehanson/XcodeColors
 * 
 * The default value if NO.
**/
@property (readwrite, assign) BOOL colorsEnabled;

/**
 * The default color set (foregroundColor, backgroundColor) is:
 * 
 * - LOG_FLAG_ERROR = (red, nil)
 * - LOG_FLAG_WARN  = (orange, nil)
 * 
 * You can customize the colors however you see fit.
 * Please note that you are passing a flag, NOT a level.
 * 
 * GOOD : [ttyLogger setForegroundColor:pink backgroundColor:nil forFlag:LOG_FLAG_INFO];  // <- Good :)
 *  BAD : [ttyLogger setForegroundColor:pink backgroundColor:nil forFlag:LOG_LEVEL_INFO]; // <- BAD! :(
 * 
 * LOG_FLAG_INFO  = 0...00100
 * LOG_LEVEL_INFO = 0...00111 <- Would match LOG_FLAG_INFO and LOG_FLAG_WARN and LOG_FLAG_ERROR
 * 
 * If you run the application within Xcode, then the XcodeColors plugin is required.
 * 
 * If you run the application from a shell, then DDTTYLogger will automatically map the given color to
 * the closest available color. (xterm-256color or xterm-color which have 256 and 16 supported colors respectively.)
 * 
 * This method invokes setForegroundColor:backgroundColor:forFlag:context: and applies it to `LOG_CONTEXT_ALL`.
**/
#if TARGET_OS_IPHONE
- (void)setForegroundColor:(UIColor *)txtColor backgroundColor:(UIColor *)bgColor forFlag:(int)mask;
#else
- (void)setForegroundColor:(NSColor *)txtColor backgroundColor:(NSColor *)bgColor forFlag:(int)mask;
#endif

/**
 * Just like setForegroundColor:backgroundColor:flag, but allows you to specify a particular logging context.
 * 
 * A logging context is often used to identify log messages coming from a 3rd party framework,
 * although logging context's can be used for many different functions.
 * 
 * Use LOG_CONTEXT_ALL to set the deafult color for all contexts that have no specific color set defined.
 * 
 * Logging context's are explained in further detail here:
 * https://github.com/CocoaLumberjack/CocoaLumberjack/wiki/CustomContext
**/
#if TARGET_OS_IPHONE
- (void)setForegroundColor:(UIColor *)txtColor backgroundColor:(UIColor *)bgColor forFlag:(int)mask context:(int)ctxt;
#else
- (void)setForegroundColor:(NSColor *)txtColor backgroundColor:(NSColor *)bgColor forFlag:(int)mask context:(int)ctxt;
#endif

/**
 * Similar to the methods above, but allows you to map DDLogMessage->tag to a particular color profile.
 * For example, you could do something like this:
 * 
 * static NSString *const PurpleTag = @"PurpleTag";
 * 
 * #define DDLogPurple(frmt, ...) LOG_OBJC_TAG_MACRO(NO, 0, 0, 0, PurpleTag, frmt, ##__VA_ARGS__)
 * 
 * And then in your applicationDidFinishLaunching, or wherever you configure Lumberjack:
 * 
 * #if TARGET_OS_IPHONE
 *   UIColor *purple = [UIColor colorWithRed:(64/255.0) green:(0/255.0) blue:(128/255.0) alpha:1.0];
 * #else
 *   NSColor *purple = [NSColor colorWithCalibratedRed:(64/255.0) green:(0/255.0) blue:(128/255.0) alpha:1.0];
 * 
 * [[DDTTYLogger sharedInstance] setForegroundColor:purple backgroundColor:nil forTag:PurpleTag];
 * [DDLog addLogger:[DDTTYLogger sharedInstance]];
 * 
 * This would essentially give you a straight NSLog replacement that prints in purple:
 * 
 * DDLogPurple(@"I'm a purple log message!");
**/
#if TARGET_OS_IPHONE
- (void)setForegroundColor:(UIColor *)txtColor backgroundColor:(UIColor *)bgColor forTag:(id <NSCopying>)tag;
#else
- (void)setForegroundColor:(NSColor *)txtColor backgroundColor:(NSColor *)bgColor forTag:(id <NSCopying>)tag;
#endif

/**
 * Clearing color profiles.
**/
- (void)clearColorsForFlag:(int)mask;
- (void)clearColorsForFlag:(int)mask context:(int)context;
- (void)clearColorsForTag:(id <NSCopying>)tag;
- (void)clearColorsForAllFlags;
- (void)clearColorsForAllTags;
- (void)clearAllColors;

@end
