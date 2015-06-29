#import "DDMultiFormatter.h"

/**
 * Welcome to Cocoa Lumberjack!
 *
 * The project page has a wealth of documentation if you have any questions.
 * https://github.com/CocoaLumberjack/CocoaLumberjack
 *
 * If you're new to the project you may wish to read the "Getting Started" page.
 * https://github.com/CocoaLumberjack/CocoaLumberjack/wiki/GettingStarted
 **/

#if TARGET_OS_IPHONE
// Compiling for iOS
#if __IPHONE_OS_VERSION_MIN_REQUIRED >= 60000 // iOS 6.0 or later
#define NEEDS_DISPATCH_RETAIN_RELEASE 0
#else                                         // iOS 5.X or earlier
#define NEEDS_DISPATCH_RETAIN_RELEASE 1
#endif
#else
// Compiling for Mac OS X
#if MAC_OS_X_VERSION_MIN_REQUIRED >= 1080     // Mac OS X 10.8 or later
#define NEEDS_DISPATCH_RETAIN_RELEASE 0
#else                                         // Mac OS X 10.7 or earlier
#define NEEDS_DISPATCH_RETAIN_RELEASE 1
#endif
#endif


@interface DDMultiFormatter ()

- (DDLogMessage *)logMessageForLine:(NSString *)line originalMessage:(DDLogMessage *)message;

@end


@implementation DDMultiFormatter {
    dispatch_queue_t _queue;
    NSMutableArray *_formatters;
}

- (id)init {
    self = [super init];
    if (self) {
#if MAC_OS_X_VERSION_MIN_REQUIRED >= 1070
        _queue = dispatch_queue_create("cocoa.lumberjack.multiformatter", DISPATCH_QUEUE_CONCURRENT);
#else
        _queue = dispatch_queue_create("cocoa.lumberjack.multiformatter", NULL);
#endif
        _formatters = [NSMutableArray new];
    }
    
    return self;
}

#if NEEDS_DISPATCH_RETAIN_RELEASE
- (void)dealloc {
    dispatch_release(_queue);
}
#endif

#pragma mark Processing

- (NSString *)formatLogMessage:(DDLogMessage *)logMessage {
    __block NSString *line = logMessage->logMsg;
    
    dispatch_sync(_queue, ^{
        for (id<DDLogFormatter> formatter in _formatters) {
            DDLogMessage *message = [self logMessageForLine:line originalMessage:logMessage];
            line = [formatter formatLogMessage:message];
            
            if (!line) {
                break;
            }
        }
    });
    
    return line;
}

- (DDLogMessage *)logMessageForLine:(NSString *)line originalMessage:(DDLogMessage *)message {
    DDLogMessage *newMessage = [message copy];
    newMessage->logMsg = line;
    return newMessage;
}

#pragma mark Formatters

- (NSArray *)formatters {
    __block NSArray *formatters;
    
    dispatch_sync(_queue, ^{
        formatters = [_formatters copy];
    });
    
    return formatters;
}

- (void)addFormatter:(id<DDLogFormatter>)formatter {
    dispatch_barrier_async(_queue, ^{
        [_formatters addObject:formatter];
    });
}

- (void)removeFormatter:(id<DDLogFormatter>)formatter {
    dispatch_barrier_async(_queue, ^{
        [_formatters removeObject:formatter];
    });
}

- (void)removeAllFormatters {
    dispatch_barrier_async(_queue, ^{
        [_formatters removeAllObjects];
    });
}

- (BOOL)isFormattingWithFormatter:(id<DDLogFormatter>)formatter {
    __block BOOL hasFormatter;
    
    dispatch_sync(_queue, ^{
        hasFormatter = [_formatters containsObject:formatter];
    });
    
    return hasFormatter;
}

@end
