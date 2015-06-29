#import "DDLog.h"

#import <pthread.h>
#import <objc/runtime.h>
#import <mach/mach_host.h>
#import <mach/host_info.h>
#import <libkern/OSAtomic.h>
#import <Availability.h>
#if TARGET_OS_IPHONE
    #import <UIKit/UIDevice.h>
#endif

/**
 * Welcome to Cocoa Lumberjack!
 * 
 * The project page has a wealth of documentation if you have any questions.
 * https://github.com/CocoaLumberjack/CocoaLumberjack
 * 
 * If you're new to the project you may wish to read the "Getting Started" wiki.
 * https://github.com/CocoaLumberjack/CocoaLumberjack/wiki/GettingStarted
 * 
**/

#if ! __has_feature(objc_arc)
#warning This file must be compiled with ARC. Use -fobjc-arc flag (or convert project to ARC).
#endif

// We probably shouldn't be using DDLog() statements within the DDLog implementation.
// But we still want to leave our log statements for any future debugging,
// and to allow other developers to trace the implementation (which is a great learning tool).
// 
// So we use a primitive logging macro around NSLog.
// We maintain the NS prefix on the macros to be explicit about the fact that we're using NSLog.

#define DD_DEBUG NO

#define NSLogDebug(frmt, ...) do{ if(DD_DEBUG) NSLog((frmt), ##__VA_ARGS__); } while(0)

// Specifies the maximum queue size of the logging thread.
// 
// Since most logging is asynchronous, its possible for rogue threads to flood the logging queue.
// That is, to issue an abundance of log statements faster than the logging thread can keepup.
// Typically such a scenario occurs when log statements are added haphazardly within large loops,
// but may also be possible if relatively slow loggers are being used.
// 
// This property caps the queue size at a given number of outstanding log statements.
// If a thread attempts to issue a log statement when the queue is already maxed out,
// the issuing thread will block until the queue size drops below the max again.

#define LOG_MAX_QUEUE_SIZE 1000 // Should not exceed INT32_MAX

// The "global logging queue" refers to [DDLog loggingQueue].
// It is the queue that all log statements go through.
//
// The logging queue sets a flag via dispatch_queue_set_specific using this key.
// We can check for this key via dispatch_get_specific() to see if we're on the "global logging queue".

static void *const GlobalLoggingQueueIdentityKey = (void *)&GlobalLoggingQueueIdentityKey;


@interface DDLoggerNode : NSObject {
@public 
    id <DDLogger> logger;   
    dispatch_queue_t loggerQueue;
    int logLevel;
}

@property (nonatomic, assign, readonly) int logLevel;

+ (DDLoggerNode *)nodeWithLogger:(id <DDLogger>)logger loggerQueue:(dispatch_queue_t)loggerQueue logLevel:(int)logLevel;

@end


@interface DDLog (PrivateAPI)

+ (void)lt_addLogger:(id <DDLogger>)logger logLevel:(int)logLevel;
+ (void)lt_removeLogger:(id <DDLogger>)logger;
+ (void)lt_removeAllLoggers;
+ (void)lt_log:(DDLogMessage *)logMessage;
+ (void)lt_flush;

@end

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma mark -
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation DDLog

// An array used to manage all the individual loggers.
// The array is only modified on the loggingQueue/loggingThread.
static NSMutableArray *loggers;

// All logging statements are added to the same queue to ensure FIFO operation.
static dispatch_queue_t loggingQueue;

// Individual loggers are executed concurrently per log statement.
// Each logger has it's own associated queue, and a dispatch group is used for synchrnoization.
static dispatch_group_t loggingGroup;

// In order to prevent to queue from growing infinitely large,
// a maximum size is enforced (LOG_MAX_QUEUE_SIZE).
static dispatch_semaphore_t queueSemaphore;

// Minor optimization for uniprocessor machines
static unsigned int numProcessors;

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
        
        loggers = [[NSMutableArray alloc] initWithCapacity:4];
        
        NSLogDebug(@"DDLog: Using grand central dispatch");
        
        loggingQueue = dispatch_queue_create("cocoa.lumberjack", NULL);
        loggingGroup = dispatch_group_create();
        
        void *nonNullValue = GlobalLoggingQueueIdentityKey; // Whatever, just not null
        dispatch_queue_set_specific(loggingQueue, GlobalLoggingQueueIdentityKey, nonNullValue, NULL);
        
        queueSemaphore = dispatch_semaphore_create(LOG_MAX_QUEUE_SIZE);
        
        // Figure out how many processors are available.
        // This may be used later for an optimization on uniprocessor machines.
        
        host_basic_info_data_t hostInfo;
        mach_msg_type_number_t infoCount;
        
        infoCount = HOST_BASIC_INFO_COUNT;
        host_info(mach_host_self(), HOST_BASIC_INFO, (host_info_t)&hostInfo, &infoCount);
        
        unsigned int result = (unsigned int)(hostInfo.max_cpus);
        unsigned int one    = (unsigned int)(1);
        
        numProcessors = MAX(result, one);
        
        NSLogDebug(@"DDLog: numProcessors = %u", numProcessors);
            
        
    #if TARGET_OS_IPHONE
        NSString *notificationName = @"UIApplicationWillTerminateNotification";
    #else
        NSString *notificationName = nil;

        if (NSApp)
        {
            notificationName = @"NSApplicationWillTerminateNotification";
        }
        else
        {
            // If there is no NSApp -> we are running Command Line Tool app.
            // In this case terminate notification wouldn't be fired, so we use workaround.
            atexit_b(^{
                [self applicationWillTerminate:nil];
            });
        }
    #endif
        
        if (notificationName) {
            [[NSNotificationCenter defaultCenter] addObserver:self
                                                     selector:@selector(applicationWillTerminate:)
                                                         name:notificationName
                                                       object:nil];
        }
    }
}

/**
 * Provides access to the logging queue.
**/
+ (dispatch_queue_t)loggingQueue
{
    return loggingQueue;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma mark Notifications
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

+ (void)applicationWillTerminate:(NSNotification *)notification
{
    [self flushLog];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma mark Logger Management
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

+ (void)addLogger:(id <DDLogger>)logger
{
    [self addLogger:logger withLogLevel:LOG_LEVEL_VERBOSE];
}

+ (void)addLogger:(id <DDLogger>)logger withLogLevel:(int)logLevel
{
    if (logger == nil) return;
    
    dispatch_async(loggingQueue, ^{ @autoreleasepool {
        
        [self lt_addLogger:logger logLevel:logLevel];
    }});
}

+ (void)removeLogger:(id <DDLogger>)logger
{
    if (logger == nil) return;
    
    dispatch_async(loggingQueue, ^{ @autoreleasepool {
        
        [self lt_removeLogger:logger];
    }});
}

+ (void)removeAllLoggers
{
    dispatch_async(loggingQueue, ^{ @autoreleasepool {
        
        [self lt_removeAllLoggers];
    }});
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma mark Master Logging
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

+ (void)queueLogMessage:(DDLogMessage *)logMessage asynchronously:(BOOL)asyncFlag
{
    // We have a tricky situation here...
    // 
    // In the common case, when the queueSize is below the maximumQueueSize,
    // we want to simply enqueue the logMessage. And we want to do this as fast as possible,
    // which means we don't want to block and we don't want to use any locks.
    // 
    // However, if the queueSize gets too big, we want to block.
    // But we have very strict requirements as to when we block, and how long we block.
    // 
    // The following example should help illustrate our requirements:
    // 
    // Imagine that the maximum queue size is configured to be 5,
    // and that there are already 5 log messages queued.
    // Let us call these 5 queued log messages A, B, C, D, and E. (A is next to be executed)
    // 
    // Now if our thread issues a log statement (let us call the log message F),
    // it should block before the message is added to the queue.
    // Furthermore, it should be unblocked immediately after A has been unqueued.
    // 
    // The requirements are strict in this manner so that we block only as long as necessary,
    // and so that blocked threads are unblocked in the order in which they were blocked.
    // 
    // Returning to our previous example, let us assume that log messages A through E are still queued.
    // Our aforementioned thread is blocked attempting to queue log message F.
    // Now assume we have another separate thread that attempts to issue log message G.
    // It should block until log messages A and B have been unqueued.
    
    
    // We are using a counting semaphore provided by GCD.
    // The semaphore is initialized with our LOG_MAX_QUEUE_SIZE value.
    // Everytime we want to queue a log message we decrement this value.
    // If the resulting value is less than zero,
    // the semaphore function waits in FIFO order for a signal to occur before returning.
    // 
    // A dispatch semaphore is an efficient implementation of a traditional counting semaphore.
    // Dispatch semaphores call down to the kernel only when the calling thread needs to be blocked.
    // If the calling semaphore does not need to block, no kernel call is made.
    
    dispatch_semaphore_wait(queueSemaphore, DISPATCH_TIME_FOREVER);
    
    // We've now sure we won't overflow the queue.
    // It is time to queue our log message.
    
    dispatch_block_t logBlock = ^{ @autoreleasepool {
        
        [self lt_log:logMessage];
    }};
    
    if (asyncFlag)
        dispatch_async(loggingQueue, logBlock);
    else
        dispatch_sync(loggingQueue, logBlock);
}

+ (void)log:(BOOL)asynchronous
      level:(int)level
       flag:(int)flag
    context:(int)context
       file:(const char *)file
   function:(const char *)function
       line:(int)line
        tag:(id)tag
     format:(NSString *)format, ...
{
    va_list args;
    if (format)
    {
        va_start(args, format);
        
        NSString *logMsg = [[NSString alloc] initWithFormat:format arguments:args];
        DDLogMessage *logMessage = [[DDLogMessage alloc] initWithLogMsg:logMsg
                                                                  level:level
                                                                   flag:flag
                                                                context:context
                                                                   file:file
                                                               function:function
                                                                   line:line
                                                                    tag:tag
                                                                options:0];
        
        [self queueLogMessage:logMessage asynchronously:asynchronous];
        
        va_end(args);
    }
}

+ (void)log:(BOOL)asynchronous
      level:(int)level
       flag:(int)flag
    context:(int)context
       file:(const char *)file
   function:(const char *)function
       line:(int)line
        tag:(id)tag
     format:(NSString *)format
       args:(va_list)args
{
    if (format)
    {
        NSString *logMsg = [[NSString alloc] initWithFormat:format arguments:args];
        DDLogMessage *logMessage = [[DDLogMessage alloc] initWithLogMsg:logMsg
                                                                  level:level
                                                                   flag:flag
                                                                context:context
                                                                   file:file
                                                               function:function
                                                                   line:line
                                                                    tag:tag
                                                                options:0];
        
        [self queueLogMessage:logMessage asynchronously:asynchronous];
    }
}

+ (void)flushLog
{
    dispatch_sync(loggingQueue, ^{ @autoreleasepool {
        
        [self lt_flush];
    }});
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma mark Registered Dynamic Logging
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

+ (BOOL)isRegisteredClass:(Class)class
{
    SEL getterSel = @selector(ddLogLevel);
    SEL setterSel = @selector(ddSetLogLevel:);
    
#if TARGET_OS_IPHONE && !TARGET_IPHONE_SIMULATOR
    
    // Issue #6 (GoogleCode) - Crashes on iOS 4.2.1 and iPhone 4
    // 
    // Crash caused by class_getClassMethod(2).
    // 
    //     "It's a bug with UIAccessibilitySafeCategory__NSObject so it didn't pop up until
    //      users had VoiceOver enabled [...]. I was able to work around it by searching the
    //      result of class_copyMethodList() instead of calling class_getClassMethod()"
    
    BOOL result = NO;
    
    unsigned int methodCount, i;
    Method *methodList = class_copyMethodList(object_getClass(class), &methodCount);
    
    if (methodList != NULL)
    {
        BOOL getterFound = NO;
        BOOL setterFound = NO;
        
        for (i = 0; i < methodCount; ++i)
        {
            SEL currentSel = method_getName(methodList[i]);
            
            if (currentSel == getterSel)
            {
                getterFound = YES;
            }
            else if (currentSel == setterSel)
            {
                setterFound = YES;
            }
            
            if (getterFound && setterFound)
            {
                result = YES;
                break;
            }
        }
        
        free(methodList);
    }
    
    return result;
    
#else
    
    // Issue #24 (GitHub) - Crashing in in ARC+Simulator
    // 
    // The method +[DDLog isRegisteredClass] will crash a project when using it with ARC + Simulator.
    // For running in the Simulator, it needs to execute the non-iOS code.
    
    Method getter = class_getClassMethod(class, getterSel);
    Method setter = class_getClassMethod(class, setterSel);
    
    if ((getter != NULL) && (setter != NULL))
    {
        return YES;
    }
    
    return NO;
    
#endif
}

+ (NSArray *)registeredClasses
{
    int numClasses, i;
    
    // We're going to get the list of all registered classes.
    // The Objective-C runtime library automatically registers all the classes defined in your source code.
    // 
    // To do this we use the following method (documented in the Objective-C Runtime Reference):
    // 
    // int objc_getClassList(Class *buffer, int bufferLen)
    // 
    // We can pass (NULL, 0) to obtain the total number of
    // registered class definitions without actually retrieving any class definitions.
    // This allows us to allocate the minimum amount of memory needed for the application.
    
    numClasses = objc_getClassList(NULL, 0);
    
    // The numClasses method now tells us how many classes we have.
    // So we can allocate our buffer, and get pointers to all the class definitions.
    
    Class *classes = (Class *)malloc(sizeof(Class) * numClasses);
    if (classes == NULL) return nil;
    
    numClasses = objc_getClassList(classes, numClasses);
    
    // We can now loop through the classes, and test each one to see if it is a DDLogging class.
    
    NSMutableArray *result = [NSMutableArray arrayWithCapacity:numClasses];
    
    for (i = 0; i < numClasses; i++)
    {
        Class class = classes[i];
        
        if ([self isRegisteredClass:class])
        {
            [result addObject:class];
        }
    }
    
    free(classes);
    
    return result;
}

+ (NSArray *)registeredClassNames
{
    NSArray *registeredClasses = [self registeredClasses];
    NSMutableArray *result = [NSMutableArray arrayWithCapacity:[registeredClasses count]];
    
    for (Class class in registeredClasses)
    {
        [result addObject:NSStringFromClass(class)];
    }
    
    return result;
}

+ (int)logLevelForClass:(Class)aClass
{
    if ([self isRegisteredClass:aClass])
    {
        return [aClass ddLogLevel];
    }
    
    return -1;
}

+ (int)logLevelForClassWithName:(NSString *)aClassName
{
    Class aClass = NSClassFromString(aClassName);
    
    return [self logLevelForClass:aClass];
}

+ (void)setLogLevel:(int)logLevel forClass:(Class)aClass
{
    if ([self isRegisteredClass:aClass])
    {
        [aClass ddSetLogLevel:logLevel];
    }
}

+ (void)setLogLevel:(int)logLevel forClassWithName:(NSString *)aClassName
{
    Class aClass = NSClassFromString(aClassName);
    
    [self setLogLevel:logLevel forClass:aClass];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma mark Logging Thread
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * This method should only be run on the logging thread/queue.
**/
+ (void)lt_addLogger:(id <DDLogger>)logger logLevel:(int)logLevel
{
    // Add to loggers array.
    // Need to create loggerQueue if loggerNode doesn't provide one.
    
    dispatch_queue_t loggerQueue = NULL;
    
    if ([logger respondsToSelector:@selector(loggerQueue)])
    {
        // Logger may be providing its own queue
        
        loggerQueue = [logger loggerQueue];
    }
    
    if (loggerQueue == nil)
    {
        // Automatically create queue for the logger.
        // Use the logger name as the queue name if possible.
        
        const char *loggerQueueName = NULL;
        if ([logger respondsToSelector:@selector(loggerName)])
        {
            loggerQueueName = [[logger loggerName] UTF8String];
        }
        
        loggerQueue = dispatch_queue_create(loggerQueueName, NULL);
    }
    
    DDLoggerNode *loggerNode = [DDLoggerNode nodeWithLogger:logger loggerQueue:loggerQueue logLevel:logLevel];
    [loggers addObject:loggerNode];
    
    if ([logger respondsToSelector:@selector(didAddLogger)])
    {
        dispatch_async(loggerNode->loggerQueue, ^{ @autoreleasepool {
            
            [logger didAddLogger];
        }});
    }
}

/**
 * This method should only be run on the logging thread/queue.
**/
+ (void)lt_removeLogger:(id <DDLogger>)logger
{
    // Find associated loggerNode in list of added loggers
    
    DDLoggerNode *loggerNode = nil;
    
    for (DDLoggerNode *node in loggers)
    {
        if (node->logger == logger)
        {
            loggerNode = node;
            break;
        }
    }
    
    if (loggerNode == nil)
    {
        NSLogDebug(@"DDLog: Request to remove logger which wasn't added");
        return;
    }
    
    // Notify logger
    
    if ([logger respondsToSelector:@selector(willRemoveLogger)])
    {
        dispatch_async(loggerNode->loggerQueue, ^{ @autoreleasepool {
            
            [logger willRemoveLogger];
        }});
    }
    
    // Remove from loggers array
    
    [loggers removeObject:loggerNode];
}

/**
 * This method should only be run on the logging thread/queue.
**/
+ (void)lt_removeAllLoggers
{
    // Notify all loggers
    
    for (DDLoggerNode *loggerNode in loggers)
    {
        if ([loggerNode->logger respondsToSelector:@selector(willRemoveLogger)])
        {
            dispatch_async(loggerNode->loggerQueue, ^{ @autoreleasepool {
                
                [loggerNode->logger willRemoveLogger];
            }});
        }
    }
    
    // Remove all loggers from array
    
    [loggers removeAllObjects];
}

/**
 * This method should only be run on the logging thread/queue.
**/
+ (void)lt_log:(DDLogMessage *)logMessage
{
    // Execute the given log message on each of our loggers.
    
    if (numProcessors > 1)
    {
        // Execute each logger concurrently, each within its own queue.
        // All blocks are added to same group.
        // After each block has been queued, wait on group.
        // 
        // The waiting ensures that a slow logger doesn't end up with a large queue of pending log messages.
        // This would defeat the purpose of the efforts we made earlier to restrict the max queue size.
        
        for (DDLoggerNode *loggerNode in loggers)
        {
            // skip the loggers that shouldn't write this message based on the logLevel

            if (logMessage->logFlag > loggerNode.logLevel)
                continue;

            dispatch_group_async(loggingGroup, loggerNode->loggerQueue, ^{ @autoreleasepool {
                
                [loggerNode->logger logMessage:logMessage];
            
            }});
        }
        
        dispatch_group_wait(loggingGroup, DISPATCH_TIME_FOREVER);
    }
    else
    {
        // Execute each logger serialy, each within its own queue.
        
        for (DDLoggerNode *loggerNode in loggers)
        {
            // skip the loggers that shouldn't write this message based on the logLevel
            
            if (logMessage->logFlag > loggerNode.logLevel)
                continue;

            dispatch_sync(loggerNode->loggerQueue, ^{ @autoreleasepool {
                
                [loggerNode->logger logMessage:logMessage];
                
            }});
        }
    }
    
    // If our queue got too big, there may be blocked threads waiting to add log messages to the queue.
    // Since we've now dequeued an item from the log, we may need to unblock the next thread.
    
    // We are using a counting semaphore provided by GCD.
    // The semaphore is initialized with our LOG_MAX_QUEUE_SIZE value.
    // When a log message is queued this value is decremented.
    // When a log message is dequeued this value is incremented.
    // If the value ever drops below zero,
    // the queueing thread blocks and waits in FIFO order for us to signal it.
    // 
    // A dispatch semaphore is an efficient implementation of a traditional counting semaphore.
    // Dispatch semaphores call down to the kernel only when the calling thread needs to be blocked.
    // If the calling semaphore does not need to block, no kernel call is made.
    
    dispatch_semaphore_signal(queueSemaphore);
}

/**
 * This method should only be run on the background logging thread.
**/
+ (void)lt_flush
{
    // All log statements issued before the flush method was invoked have now been executed.
    // 
    // Now we need to propogate the flush request to any loggers that implement the flush method.
    // This is designed for loggers that buffer IO.
        
    for (DDLoggerNode *loggerNode in loggers)
    {
        if ([loggerNode->logger respondsToSelector:@selector(flush)])
        {
            dispatch_group_async(loggingGroup, loggerNode->loggerQueue, ^{ @autoreleasepool {
                
                [loggerNode->logger flush];
                
            }});
        }
    }
    
    dispatch_group_wait(loggingGroup, DISPATCH_TIME_FOREVER);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma mark Utilities
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

NSString *DDExtractFileNameWithoutExtension(const char *filePath, BOOL copy)
{
    if (filePath == NULL) return nil;
    
    char *lastSlash = NULL;
    char *lastDot = NULL;
    
    char *p = (char *)filePath;
    
    while (*p != '\0')
    {
        if (*p == '/')
            lastSlash = p;
        else if (*p == '.')
            lastDot = p;
        
        p++;
    }
    
    char *subStr;
    NSUInteger subLen;
    
    if (lastSlash)
    {
        if (lastDot)
        {
            // lastSlash -> lastDot
            subStr = lastSlash + 1;
            subLen = lastDot - subStr;
        }
        else
        {
            // lastSlash -> endOfString
            subStr = lastSlash + 1;
            subLen = p - subStr;
        }
    }
    else
    {
        if (lastDot)
        {
            // startOfString -> lastDot
            subStr = (char *)filePath;
            subLen = lastDot - subStr;
        }
        else
        {
            // startOfString -> endOfString
            subStr = (char *)filePath;
            subLen = p - subStr;
        }
    }
    
    if (copy)
    {
        return [[NSString alloc] initWithBytes:subStr
                                        length:subLen
                                      encoding:NSUTF8StringEncoding];
    }
    else
    {
        // We can take advantage of the fact that __FILE__ is a string literal.
        // Specifically, we don't need to waste time copying the string.
        // We can just tell NSString to point to a range within the string literal.
        
        return [[NSString alloc] initWithBytesNoCopy:subStr
                                              length:subLen
                                            encoding:NSUTF8StringEncoding
                                        freeWhenDone:NO];
    }
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma mark -
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation DDLoggerNode

@synthesize logLevel;

- (instancetype)initWithLogger:(id <DDLogger>)aLogger loggerQueue:(dispatch_queue_t)aLoggerQueue logLevel:(int)aLogLevel
{
    if ((self = [super init]))
    {
        logger = aLogger;
        
        if (aLoggerQueue) {
            loggerQueue = aLoggerQueue;
            #if !OS_OBJECT_USE_OBJC
            dispatch_retain(loggerQueue);
            #endif
        }
        logLevel = aLogLevel;
    }
    return self;
}

+ (DDLoggerNode *)nodeWithLogger:(id <DDLogger>)logger loggerQueue:(dispatch_queue_t)loggerQueue logLevel:(int)logLevel
{
    return [[DDLoggerNode alloc] initWithLogger:logger loggerQueue:loggerQueue logLevel:logLevel];
}

- (void)dealloc
{
    #if !OS_OBJECT_USE_OBJC
    if (loggerQueue) dispatch_release(loggerQueue);
    #endif
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma mark -
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation DDLogMessage

static char *dd_str_copy(const char *str)
{
    if (str == NULL) return NULL;
    
    size_t length = strlen(str);
    char * result = malloc(length + 1);
    if (result == NULL) return NULL;
    strncpy(result, str, length);
    result[length] = 0;
    
    return result;
}

- (instancetype)initWithLogMsg:(NSString *)msg
                         level:(int)level
                          flag:(int)flag
                       context:(int)context
                          file:(const char *)aFile
                      function:(const char *)aFunction
                          line:(int)line
                           tag:(id)aTag
                       options:(DDLogMessageOptions)optionsMask
{
    if ((self = [super init]))
    {
        logMsg     = msg;
        logLevel   = level;
        logFlag    = flag;
        logContext = context;
        lineNumber = line;
        tag        = aTag;
        options    = optionsMask;
        
        if (options & DDLogMessageCopyFile)
            file = dd_str_copy(aFile);
        else
            file = (char *)aFile;
        
        if (options & DDLogMessageCopyFunction)
            function = dd_str_copy(aFunction);
        else
            function = (char *)aFunction;
        
        timestamp = [[NSDate alloc] init];
        
        machThreadID = pthread_mach_thread_np(pthread_self());

        // Try to get the current queue's label
        
        // a) Compiling against newer SDK's (iOS 7+/OS X 10.9+) where DISPATCH_CURRENT_QUEUE_LABEL is defined
        //    on a (iOS 7.0+/OS X 10.9+) runtime version
        BOOL gotLabel = NO;
        #ifdef DISPATCH_CURRENT_QUEUE_LABEL
        if (
            #if TARGET_OS_IPHONE
                #ifndef NSFoundationVersionNumber_iOS_6_1
                #define NSFoundationVersionNumber_iOS_6_1 993.00
                #endif
                floor(NSFoundationVersionNumber) > NSFoundationVersionNumber_iOS_6_1 // iOS 7+ (> iOS 6.1)
            #else
                [[NSApplication sharedApplication] respondsToSelector:@selector(occlusionState)] // OS X 10.9+
            #endif
            ) {
            queueLabel = dd_str_copy(dispatch_queue_get_label(DISPATCH_CURRENT_QUEUE_LABEL));
            gotLabel = YES;
        }
        #endif
        
        // b) Systems where dispatch_get_current_queue is not yet deprecated and won't crash (< iOS 6.0/OS X 10.9)
        //    dispatch_get_current_queue(void); __OSX_AVAILABLE_BUT_DEPRECATED(__MAC_10_6,__MAC_10_9,__IPHONE_4_0,__IPHONE_6_0)
        if (!gotLabel &&
        #if TARGET_OS_IPHONE
            #ifndef NSFoundationVersionNumber_iOS_6_0
            #define NSFoundationVersionNumber_iOS_6_0 993.00
            #endif
            floor(NSFoundationVersionNumber) < NSFoundationVersionNumber_iOS_6_0 // < iOS 6.0
        #else
            ![[NSApplication sharedApplication] respondsToSelector:@selector(occlusionState)] // < OS X 10.9
        #endif
            ) {
            #pragma clang diagnostic push
            #pragma clang diagnostic ignored "-Wdeprecated-declarations"
            dispatch_queue_t currentQueue = dispatch_get_current_queue();
            #pragma clang diagnostic pop
            
            queueLabel = dd_str_copy(dispatch_queue_get_label(currentQueue));
            gotLabel = YES;
        }
        
        // c) Give up
        if (!gotLabel) {
            queueLabel = dd_str_copy(""); // iOS 6.x only
        }
        
        threadName = [[NSThread currentThread] name];
    }
    return self;
}

- (NSString *)threadID
{
    return [[NSString alloc] initWithFormat:@"%x", machThreadID];
}

- (NSString *)fileName
{
    return DDExtractFileNameWithoutExtension(file, NO);
}

- (NSString *)methodName
{
    if (function == NULL)
        return nil;
    else
        return [[NSString alloc] initWithUTF8String:function];
}

- (void)dealloc
{
    if (file && (options & DDLogMessageCopyFile))
        free(file);
    
    if (function && (options & DDLogMessageCopyFunction))
        free(function);
    
    if (queueLabel)
        free(queueLabel);
}


- (id)copyWithZone:(NSZone *)zone {
    DDLogMessage *newMessage = [[DDLogMessage alloc] init];
    
    newMessage->logLevel = self->logLevel;
    newMessage->logFlag = self->logFlag;
    newMessage->logContext = self->logContext;
    newMessage->logMsg = self->logMsg;
    newMessage->timestamp = self->timestamp;
    
    if (self->options & DDLogMessageCopyFile) {
        newMessage->file = dd_str_copy(self->file);
        newMessage->function = dd_str_copy(self->function);
    } else {
        newMessage->file = self->file;
        newMessage->function = self->function;
    }
    
    newMessage->lineNumber = self->lineNumber;
    
    newMessage->machThreadID = self->machThreadID;
    newMessage->queueLabel = dd_str_copy(self->queueLabel);
    newMessage->threadName = self->threadName;
    newMessage->tag = self->tag;
    newMessage->options = self->options;
    
    return newMessage;
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma mark -
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation DDAbstractLogger

- (id)init
{
    if ((self = [super init]))
    {
        const char *loggerQueueName = NULL;
        if ([self respondsToSelector:@selector(loggerName)])
        {
            loggerQueueName = [[self loggerName] UTF8String];
        }
        
        loggerQueue = dispatch_queue_create(loggerQueueName, NULL);
        
        // We're going to use dispatch_queue_set_specific() to "mark" our loggerQueue.
        // Later we can use dispatch_get_specific() to determine if we're executing on our loggerQueue.
        // The documentation states:
        //
        // > Keys are only compared as pointers and are never dereferenced.
        // > Thus, you can use a pointer to a static variable for a specific subsystem or
        // > any other value that allows you to identify the value uniquely.
        // > Specifying a pointer to a string constant is not recommended.
        //
        // So we're going to use the very convenient key of "self",
        // which also works when multiple logger classes extend this class, as each will have a different "self" key.
        //
        // This is used primarily for thread-safety assertions (via the isOnInternalLoggerQueue method below).
        
        void *key = (__bridge void *)self;
        void *nonNullValue = (__bridge void *)self;
        
        dispatch_queue_set_specific(loggerQueue, key, nonNullValue, NULL);
    }
    return self;
}

- (void)dealloc
{
    #if !OS_OBJECT_USE_OBJC
    if (loggerQueue) dispatch_release(loggerQueue);
    #endif
}

- (void)logMessage:(DDLogMessage *)logMessage
{
    // Override me
}

- (id <DDLogFormatter>)logFormatter
{
    // This method must be thread safe and intuitive.
    // Therefore if somebody executes the following code:
    // 
    // [logger setLogFormatter:myFormatter];
    // formatter = [logger logFormatter];
    // 
    // They would expect formatter to equal myFormatter.
    // This functionality must be ensured by the getter and setter method.
    //
    // The thread safety must not come at a cost to the performance of the logMessage method.
    // This method is likely called sporadically, while the logMessage method is called repeatedly.
    // This means, the implementation of this method:
    // - Must NOT require the logMessage method to acquire a lock.
    // - Must NOT require the logMessage method to access an atomic property (also a lock of sorts).
    //
    // Thread safety is ensured by executing access to the formatter variable on the loggerQueue.
    // This is the same queue that the logMessage method operates on.
    //
    // Note: The last time I benchmarked the performance of direct access vs atomic property access,
    // direct access was over twice as fast on the desktop and over 6 times as fast on the iPhone.
    // 
    // Furthermore, consider the following code:
    //
    // DDLogVerbose(@"log msg 1");
    // DDLogVerbose(@"log msg 2");
    // [logger setFormatter:myFormatter];
    // DDLogVerbose(@"log msg 3");
    //
    // Our intuitive requirement means that the new formatter will only apply to the 3rd log message.
    // This must remain true even when using asynchronous logging.
    // We must keep in mind the various queue's that are in play here:
    // 
    // loggerQueue : Our own private internal queue that the logMessage method runs on.
    //               Operations are added to this queue from the global loggingQueue.
    // 
    // globalLoggingQueue : The queue that all log messages go through before they arrive in our loggerQueue.
    // 
    // All log statements go through the serial gloabalLoggingQueue before they arrive at our loggerQueue.
    // Thus this method also goes through the serial globalLoggingQueue to ensure intuitive operation.
    
    // IMPORTANT NOTE:
    // 
    // Methods within the DDLogger implementation MUST access the formatter ivar directly.
    // This method is designed explicitly for external access.
    //
    // Using "self." syntax to go through this method will cause immediate deadlock.
    // This is the intended result. Fix it by accessing the ivar directly.
    // Great strides have been take to ensure this is safe to do. Plus it's MUCH faster.
    
    NSAssert(![self isOnGlobalLoggingQueue], @"Core architecture requirement failure");
    NSAssert(![self isOnInternalLoggerQueue], @"MUST access ivar directly, NOT via self.* syntax.");
    
    dispatch_queue_t globalLoggingQueue = [DDLog loggingQueue];
    
    __block id <DDLogFormatter> result;
    
    dispatch_sync(globalLoggingQueue, ^{
        dispatch_sync(loggerQueue, ^{
            result = formatter;
        });
    });
    
    return result;
}

- (void)setLogFormatter:(id <DDLogFormatter>)logFormatter
{
    // The design of this method is documented extensively in the logFormatter message (above in code).
    
    NSAssert(![self isOnGlobalLoggingQueue], @"Core architecture requirement failure");
    NSAssert(![self isOnInternalLoggerQueue], @"MUST access ivar directly, NOT via self.* syntax.");
    
    dispatch_block_t block = ^{ @autoreleasepool {
        
        if (formatter != logFormatter)
        {
            if ([formatter respondsToSelector:@selector(willRemoveFromLogger:)]) {
                [formatter willRemoveFromLogger:self];
            }
            
            formatter = logFormatter;
            
            if ([formatter respondsToSelector:@selector(didAddToLogger:)]) {
                [formatter didAddToLogger:self];
            }
        }
    }};
    
    dispatch_queue_t globalLoggingQueue = [DDLog loggingQueue];
    
    dispatch_async(globalLoggingQueue, ^{
        dispatch_async(loggerQueue, block);
    });
}

- (dispatch_queue_t)loggerQueue
{
    return loggerQueue;
}

- (NSString *)loggerName
{
    return NSStringFromClass([self class]);
}

- (BOOL)isOnGlobalLoggingQueue
{
    return (dispatch_get_specific(GlobalLoggingQueueIdentityKey) != NULL);
}

- (BOOL)isOnInternalLoggerQueue
{
    void *key = (__bridge void *)self;
    return (dispatch_get_specific(key) != NULL);
}

@end
