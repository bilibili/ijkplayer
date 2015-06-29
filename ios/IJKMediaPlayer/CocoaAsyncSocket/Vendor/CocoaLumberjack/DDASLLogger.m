#import "DDASLLogger.h"

#import <libkern/OSAtomic.h>

/**
 * Welcome to Cocoa Lumberjack!
 * 
 * The project page has a wealth of documentation if you have any questions.
 * https://github.com/CocoaLumberjack/CocoaLumberjack
 * 
 * If you're new to the project you may wish to read the "Getting Started" wiki.
 * https://github.com/CocoaLumberjack/CocoaLumberjack/wiki/GettingStarted
**/

#if ! __has_feature(objc_arc)
#warning This file must be compiled with ARC. Use -fobjc-arc flag (or convert project to ARC).
#endif


@implementation DDASLLogger

static DDASLLogger *sharedInstance;

/**
 * The runtime sends initialize to each class in a program exactly one time just before the class,
 * or any class that inherits from it, is sent its first message from within the program. (Thus the
 * method may never be invoked if the class is not used.) The runtime sends the initialize message to
 * classes in a thread-safe manner. Superclasses receive this message before their subclasses.
 *
 * This method may also be called directly (assumably by accident), hence the safety mechanism.
**/
+ (void)initialize
{
    static BOOL initialized = NO;
    if (!initialized)
    {
        initialized = YES;
        
        sharedInstance = [[[self class] alloc] init];
    }
}

+ (instancetype)sharedInstance
{
    return sharedInstance;
}

- (id)init
{
    if (sharedInstance != nil)
    {
        return nil;
    }
    
    if ((self = [super init]))
    {
        // A default asl client is provided for the main thread,
        // but background threads need to create their own client.
        
        client = asl_open(NULL, "com.apple.console", 0);
    }
    return self;
}

- (void)logMessage:(DDLogMessage *)logMessage
{
    NSString *logMsg = logMessage->logMsg;
    
    if (formatter)
    {
        logMsg = [formatter formatLogMessage:logMessage];
    }
    
    if (logMsg)
    {
        const char *msg = [logMsg UTF8String];
        
        int aslLogLevel;
        switch (logMessage->logFlag)
        {
            // Note: By default ASL will filter anything above level 5 (Notice).
            // So our mappings shouldn't go above that level.
            
            case LOG_FLAG_ERROR : aslLogLevel = ASL_LEVEL_ALERT;   break;
            case LOG_FLAG_WARN  : aslLogLevel = ASL_LEVEL_CRIT;    break;
            case LOG_FLAG_INFO  : aslLogLevel = ASL_LEVEL_ERR;     break;
            case LOG_FLAG_DEBUG : aslLogLevel = ASL_LEVEL_WARNING; break;
            default             : aslLogLevel = ASL_LEVEL_NOTICE;  break;
        }
        
        asl_log(client, NULL, aslLogLevel, "%s", msg);
    }
}

- (NSString *)loggerName
{
    return @"cocoa.lumberjack.aslLogger";
}

@end
