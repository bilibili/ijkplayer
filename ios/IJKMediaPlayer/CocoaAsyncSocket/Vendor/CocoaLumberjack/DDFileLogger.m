#import "DDFileLogger.h"

#import <unistd.h>
#import <sys/attr.h>
#import <sys/xattr.h>
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

// We probably shouldn't be using DDLog() statements within the DDLog implementation.
// But we still want to leave our log statements for any future debugging,
// and to allow other developers to trace the implementation (which is a great learning tool).
// 
// So we use primitive logging macros around NSLog.
// We maintain the NS prefix on the macros to be explicit about the fact that we're using NSLog.

#define LOG_LEVEL 2

#define NSLogError(frmt, ...)    do{ if(LOG_LEVEL >= 1) NSLog((frmt), ##__VA_ARGS__); } while(0)
#define NSLogWarn(frmt, ...)     do{ if(LOG_LEVEL >= 2) NSLog((frmt), ##__VA_ARGS__); } while(0)
#define NSLogInfo(frmt, ...)     do{ if(LOG_LEVEL >= 3) NSLog((frmt), ##__VA_ARGS__); } while(0)
#define NSLogDebug(frmt, ...)    do{ if(LOG_LEVEL >= 4) NSLog((frmt), ##__VA_ARGS__); } while(0)
#define NSLogVerbose(frmt, ...)  do{ if(LOG_LEVEL >= 5) NSLog((frmt), ##__VA_ARGS__); } while(0)

@interface DDLogFileManagerDefault (PrivateAPI)

- (void)deleteOldLogFiles;
- (NSString *)defaultLogsDirectory;

@end

@interface DDFileLogger (PrivateAPI)

- (void)rollLogFileNow;
- (void)maybeRollLogFileDueToAge;
- (void)maybeRollLogFileDueToSize;

@end

#if TARGET_OS_IPHONE
BOOL doesAppRunInBackground(void);
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma mark -
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation DDLogFileManagerDefault

@synthesize maximumNumberOfLogFiles;

- (id)init
{
    return [self initWithLogsDirectory:nil];
}

- (instancetype)initWithLogsDirectory:(NSString *)aLogsDirectory
{
    if ((self = [super init]))
    {
        maximumNumberOfLogFiles = DEFAULT_LOG_MAX_NUM_LOG_FILES;
        
        if (aLogsDirectory)
            _logsDirectory = [aLogsDirectory copy];
        else
            _logsDirectory = [[self defaultLogsDirectory] copy];
        
        NSKeyValueObservingOptions kvoOptions = NSKeyValueObservingOptionOld | NSKeyValueObservingOptionNew;
        
        [self addObserver:self forKeyPath:NSStringFromSelector(@selector(maximumNumberOfLogFiles)) options:kvoOptions context:nil];
        
        NSLogVerbose(@"DDFileLogManagerDefault: logsDirectory:\n%@", [self logsDirectory]);
        NSLogVerbose(@"DDFileLogManagerDefault: sortedLogFileNames:\n%@", [self sortedLogFileNames]);
    }
    return self;
}

- (void)dealloc
{
    // try-catch because the observer might be removed or never added. In this case, removeObserver throws and exception
    @try {
        [self removeObserver:self forKeyPath:NSStringFromSelector(@selector(maximumNumberOfLogFiles))];
    }
    @catch (NSException *exception) {
        
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma mark Configuration
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void)observeValueForKeyPath:(NSString *)keyPath
                      ofObject:(id)object
                        change:(NSDictionary *)change
                       context:(void *)context
{
    NSNumber *old = [change objectForKey:NSKeyValueChangeOldKey];
    NSNumber *new = [change objectForKey:NSKeyValueChangeNewKey];
    
    if ([old isEqual:new])
    {
        // No change in value - don't bother with any processing.
        return;
    }
    
    if ([keyPath isEqualToString:NSStringFromSelector(@selector(maximumNumberOfLogFiles))])
    {
        NSLogInfo(@"DDFileLogManagerDefault: Responding to configuration change: maximumNumberOfLogFiles");
        
        dispatch_async([DDLog loggingQueue], ^{ @autoreleasepool {
            
            [self deleteOldLogFiles];
        }});
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma mark File Deleting
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Deletes archived log files that exceed the maximumNumberOfLogFiles configuration value.
**/
- (void)deleteOldLogFiles
{
    NSLogVerbose(@"DDLogFileManagerDefault: deleteOldLogFiles");
    
    NSUInteger maxNumLogFiles = self.maximumNumberOfLogFiles;
    if (maxNumLogFiles == 0)
    {
        // Unlimited - don't delete any log files
        return;
    }
    
    NSArray *sortedLogFileInfos = [self sortedLogFileInfos];
    
    // Do we consider the first file?
    // We are only supposed to be deleting archived files.
    // In most cases, the first file is likely the log file that is currently being written to.
    // So in most cases, we do not want to consider this file for deletion.
    
    NSUInteger count = [sortedLogFileInfos count];
    BOOL excludeFirstFile = NO;
    
    if (count > 0)
    {
        DDLogFileInfo *logFileInfo = [sortedLogFileInfos objectAtIndex:0];
        
        if (!logFileInfo.isArchived)
        {
            excludeFirstFile = YES;
        }
    }
    
    NSArray *sortedArchivedLogFileInfos;
    if (excludeFirstFile)
    {
        count--;
        sortedArchivedLogFileInfos = [sortedLogFileInfos subarrayWithRange:NSMakeRange(1, count)];
    }
    else
    {
        sortedArchivedLogFileInfos = sortedLogFileInfos;
    }
    
    NSUInteger i;
    for (i = maxNumLogFiles; i < count; i++)
    {
        DDLogFileInfo *logFileInfo = [sortedArchivedLogFileInfos objectAtIndex:i];
        
        NSLogInfo(@"DDLogFileManagerDefault: Deleting file: %@", logFileInfo.fileName);
        
        [[NSFileManager defaultManager] removeItemAtPath:logFileInfo.filePath error:nil];
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma mark Log Files
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Returns the path to the default logs directory.
 * If the logs directory doesn't exist, this method automatically creates it.
**/
- (NSString *)defaultLogsDirectory
{
#if TARGET_OS_IPHONE
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);
    NSString *baseDir = ([paths count] > 0) ? [paths objectAtIndex:0] : nil;
    NSString *logsDirectory = [baseDir stringByAppendingPathComponent:@"Logs"];
    
#else
    NSString *appName = [[NSProcessInfo processInfo] processName];
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES);
    NSString *basePath = ([paths count] > 0) ? [paths objectAtIndex:0] : NSTemporaryDirectory();
    NSString *logsDirectory = [[basePath stringByAppendingPathComponent:@"Logs"] stringByAppendingPathComponent:appName];

#endif

    return logsDirectory;
}

- (NSString *)logsDirectory
{
    // We could do this check once, during initalization, and not bother again.
    // But this way the code continues to work if the directory gets deleted while the code is running.
    
    if (![[NSFileManager defaultManager] fileExistsAtPath:_logsDirectory])
    {
        NSError *err = nil;
        if (![[NSFileManager defaultManager] createDirectoryAtPath:_logsDirectory
                                       withIntermediateDirectories:YES attributes:nil error:&err])
        {
            NSLogError(@"DDFileLogManagerDefault: Error creating logsDirectory: %@", err);
        }
    }
    
    return _logsDirectory;
}

/**
 * A log file has a name like "<app name> <date> <time>.log".
 * Example: MobileSafari 2013-12-03 17-14.log
**/
- (BOOL)isLogFile:(NSString *)fileName
{
    NSString *appName = [self applicationName];

    BOOL hasProperPrefix = [fileName hasPrefix:appName];
    BOOL hasProperSuffix = [fileName hasSuffix:@".log"];
    BOOL hasProperDate = NO;

    if (hasProperPrefix && hasProperSuffix)
    {
        NSUInteger lengthOfMiddle = fileName.length - appName.length - @".log".length;

        // Date string should have at least 16 characters - " 2013-12-03 17-14"
        if (lengthOfMiddle >= 17)
        {
            NSRange range = NSMakeRange(appName.length, lengthOfMiddle);

            NSString *middle = [fileName substringWithRange:range];
            NSArray *components = [middle componentsSeparatedByString:@" "];

            // When creating logfile if there is existing file with the same name, we append attemp number at the end.
            // Thats why here we can have three or four components. For details see generateLogFileNameWithAttempt: method.
            //
            // Components:
            //     "", "2013-12-03", "17-14"
            // or
            //     "", "2013-12-03", "17-14", "1"
            if (components.count == 3 || components.count == 4)
            {
                NSString *dateString = [NSString stringWithFormat:@"%@ %@", components[1], components[2]];
                NSDateFormatter *dateFormatter = [self logFileDateFormatter];

                NSDate *date = [dateFormatter dateFromString:dateString];

                if (date)
                {
                    hasProperDate = YES;
                }
            }
        }
    }

    return (hasProperPrefix && hasProperDate && hasProperSuffix);
}

- (NSDateFormatter *)logFileDateFormatter
{
    NSDateFormatter *dateFormatter = [[NSDateFormatter alloc] init];
    [dateFormatter setDateFormat:@"yyyy'-'MM'-'dd' 'HH'-'mm'"];
    [dateFormatter setTimeZone:[NSTimeZone timeZoneForSecondsFromGMT:0]];

    return dateFormatter;
}

/**
 * Returns an array of NSString objects,
 * each of which is the filePath to an existing log file on disk.
**/
- (NSArray *)unsortedLogFilePaths
{
    NSString *logsDirectory = [self logsDirectory];
    NSArray *fileNames = [[NSFileManager defaultManager] contentsOfDirectoryAtPath:logsDirectory error:nil];
    
    NSMutableArray *unsortedLogFilePaths = [NSMutableArray arrayWithCapacity:[fileNames count]];
    
    for (NSString *fileName in fileNames)
    {
        // Filter out any files that aren't log files. (Just for extra safety)
        
        if ([self isLogFile:fileName])
        {
            NSString *filePath = [logsDirectory stringByAppendingPathComponent:fileName];
            
            [unsortedLogFilePaths addObject:filePath];
        }
    }
    
    return unsortedLogFilePaths;
}

/**
 * Returns an array of NSString objects,
 * each of which is the fileName of an existing log file on disk.
**/
- (NSArray *)unsortedLogFileNames
{
    NSArray *unsortedLogFilePaths = [self unsortedLogFilePaths];
    
    NSMutableArray *unsortedLogFileNames = [NSMutableArray arrayWithCapacity:[unsortedLogFilePaths count]];
    
    for (NSString *filePath in unsortedLogFilePaths)
    {
        [unsortedLogFileNames addObject:[filePath lastPathComponent]];
    }
    
    return unsortedLogFileNames;
}

/**
 * Returns an array of DDLogFileInfo objects,
 * each representing an existing log file on disk,
 * and containing important information about the log file such as it's modification date and size.
**/
- (NSArray *)unsortedLogFileInfos
{
    NSArray *unsortedLogFilePaths = [self unsortedLogFilePaths];
    
    NSMutableArray *unsortedLogFileInfos = [NSMutableArray arrayWithCapacity:[unsortedLogFilePaths count]];
    
    for (NSString *filePath in unsortedLogFilePaths)
    {
        DDLogFileInfo *logFileInfo = [[DDLogFileInfo alloc] initWithFilePath:filePath];
        
        [unsortedLogFileInfos addObject:logFileInfo];
    }
    
    return unsortedLogFileInfos;
}

/**
 * Just like the unsortedLogFilePaths method, but sorts the array.
 * The items in the array are sorted by creation date.
 * The first item in the array will be the most recently created log file.
**/
- (NSArray *)sortedLogFilePaths
{
    NSArray *sortedLogFileInfos = [self sortedLogFileInfos];
    
    NSMutableArray *sortedLogFilePaths = [NSMutableArray arrayWithCapacity:[sortedLogFileInfos count]];
    
    for (DDLogFileInfo *logFileInfo in sortedLogFileInfos)
    {
        [sortedLogFilePaths addObject:[logFileInfo filePath]];
    }
    
    return sortedLogFilePaths;
}

/**
 * Just like the unsortedLogFileNames method, but sorts the array.
 * The items in the array are sorted by creation date.
 * The first item in the array will be the most recently created log file.
**/
- (NSArray *)sortedLogFileNames
{
    NSArray *sortedLogFileInfos = [self sortedLogFileInfos];
    
    NSMutableArray *sortedLogFileNames = [NSMutableArray arrayWithCapacity:[sortedLogFileInfos count]];
    
    for (DDLogFileInfo *logFileInfo in sortedLogFileInfos)
    {
        [sortedLogFileNames addObject:[logFileInfo fileName]];
    }
    
    return sortedLogFileNames;
}

/**
 * Just like the unsortedLogFileInfos method, but sorts the array.
 * The items in the array are sorted by creation date.
 * The first item in the array will be the most recently created log file.
**/
- (NSArray *)sortedLogFileInfos
{
    return [[self unsortedLogFileInfos] sortedArrayUsingSelector:@selector(reverseCompareByCreationDate:)];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma mark Creation
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Generates log file name with format "<app name> <date> <time>.log"
 * Example: MobileSafari 2013-12-03 17-14.log
**/
- (NSString *)generateLogFileNameWithAttempt:(NSUInteger)attempt
{
    NSString *appName = [self applicationName];

    NSDateFormatter *dateFormatter = [self logFileDateFormatter];
    NSString *formattedDate = [dateFormatter stringFromDate:[NSDate date]];

    if (attempt > 1)
    {
        return [NSString stringWithFormat:@"%@ %@ %lu.log", appName, formattedDate, (unsigned long)attempt];
    }
    else
    {
        return [NSString stringWithFormat:@"%@ %@.log", appName, formattedDate];
    }
}

/**
 * Generates a new unique log file path, and creates the corresponding log file.
**/
- (NSString *)createNewLogFile
{
    // Generate a random log file name, and create the file (if there isn't a collision)
    
    NSString *logsDirectory = [self logsDirectory];
    NSUInteger attempt = 1;
    do
    {
        NSString *fileName = [self generateLogFileNameWithAttempt:attempt];
        
        NSString *filePath = [logsDirectory stringByAppendingPathComponent:fileName];
        
        if (![[NSFileManager defaultManager] fileExistsAtPath:filePath])
        {
            NSLogVerbose(@"DDLogFileManagerDefault: Creating new log file: %@", fileName);

            NSDictionary *attributes = nil;

        #if TARGET_OS_IPHONE
             // When creating log file on iOS we're setting NSFileProtectionKey attribute to NSFileProtectionCompleteUnlessOpen.
             // 
             // But in case if app is able to launch from background we need to have an ability to open log file any time we
             // want (even if device is locked). Thats why that attribute have to be changed to
             // NSFileProtectionCompleteUntilFirstUserAuthentication.

            NSString *key = doesAppRunInBackground() ?
                NSFileProtectionCompleteUntilFirstUserAuthentication : NSFileProtectionCompleteUnlessOpen;

            attributes = @{ NSFileProtectionKey : key };
        #endif
            
            [[NSFileManager defaultManager] createFileAtPath:filePath contents:nil attributes:attributes];
            
            // Since we just created a new log file, we may need to delete some old log files
            [self deleteOldLogFiles];
            
            return filePath;
        } else {
            attempt++;
        }
        
    } while(YES);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma mark Utility
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

- (NSString *)applicationName
{
    static NSString *_appName;
    static dispatch_once_t onceToken;

    dispatch_once(&onceToken, ^{
    #if TARGET_OS_IPHONE
        _appName = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleDisplayName"];

        if (! _appName)
        {
            _appName = [[NSProcessInfo processInfo] processName];
        }
    #else
        _appName = [[NSProcessInfo processInfo] processName];
    #endif

        if (! _appName)
        {
            _appName = @"";
        }
    });

    return _appName;
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma mark -
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation DDLogFileFormatterDefault

- (id)init
{
    return [self initWithDateFormatter:nil];
}

- (instancetype)initWithDateFormatter:(NSDateFormatter *)aDateFormatter
{
    if ((self = [super init]))
    {
        if (aDateFormatter)
        {
            dateFormatter = aDateFormatter;
        }
        else
        {
            dateFormatter = [[NSDateFormatter alloc] init];
            [dateFormatter setFormatterBehavior:NSDateFormatterBehavior10_4]; // 10.4+ style
            [dateFormatter setDateFormat:@"yyyy/MM/dd HH:mm:ss:SSS"];
        }
    }
    return self;
}

- (NSString *)formatLogMessage:(DDLogMessage *)logMessage
{
    NSString *dateAndTime = [dateFormatter stringFromDate:(logMessage->timestamp)];
    
    return [NSString stringWithFormat:@"%@  %@", dateAndTime, logMessage->logMsg];
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma mark -
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation DDFileLogger

- (id)init
{
    DDLogFileManagerDefault *defaultLogFileManager = [[DDLogFileManagerDefault alloc] init];
    
    return [self initWithLogFileManager:defaultLogFileManager];
}

- (instancetype)initWithLogFileManager:(id <DDLogFileManager>)aLogFileManager
{
    if ((self = [super init]))
    {
        maximumFileSize = DEFAULT_LOG_MAX_FILE_SIZE;
        rollingFrequency = DEFAULT_LOG_ROLLING_FREQUENCY;
        
        logFileManager = aLogFileManager;
        
        formatter = [[DDLogFileFormatterDefault alloc] init];
    }
    return self;
}

- (void)dealloc
{
    [currentLogFileHandle synchronizeFile];
    [currentLogFileHandle closeFile];

    if (currentLogFileVnode) {
        dispatch_source_cancel(currentLogFileVnode);
        currentLogFileVnode = NULL;
    }

    if (rollingTimer)
    {
        dispatch_source_cancel(rollingTimer);
        rollingTimer = NULL;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma mark Properties
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

@synthesize logFileManager;

- (unsigned long long)maximumFileSize
{
    __block unsigned long long result;
    
    dispatch_block_t block = ^{
        result = maximumFileSize;
    };
    
    // The design of this method is taken from the DDAbstractLogger implementation.
    // For extensive documentation please refer to the DDAbstractLogger implementation.
    
    // Note: The internal implementation MUST access the maximumFileSize variable directly,
    // This method is designed explicitly for external access.
    //
    // Using "self." syntax to go through this method will cause immediate deadlock.
    // This is the intended result. Fix it by accessing the ivar directly.
    // Great strides have been take to ensure this is safe to do. Plus it's MUCH faster.
    
    NSAssert(![self isOnGlobalLoggingQueue], @"Core architecture requirement failure");
    NSAssert(![self isOnInternalLoggerQueue], @"MUST access ivar directly, NOT via self.* syntax.");
    
    dispatch_queue_t globalLoggingQueue = [DDLog loggingQueue];
    
    dispatch_sync(globalLoggingQueue, ^{
        dispatch_sync(loggerQueue, block);
    });
    
    return result;
}

- (void)setMaximumFileSize:(unsigned long long)newMaximumFileSize
{
    dispatch_block_t block = ^{ @autoreleasepool {
        
        maximumFileSize = newMaximumFileSize;
        [self maybeRollLogFileDueToSize];
        
    }};
    
    // The design of this method is taken from the DDAbstractLogger implementation.
    // For extensive documentation please refer to the DDAbstractLogger implementation.
    
    // Note: The internal implementation MUST access the maximumFileSize variable directly,
    // This method is designed explicitly for external access.
    //
    // Using "self." syntax to go through this method will cause immediate deadlock.
    // This is the intended result. Fix it by accessing the ivar directly.
    // Great strides have been take to ensure this is safe to do. Plus it's MUCH faster.
    
    NSAssert(![self isOnGlobalLoggingQueue], @"Core architecture requirement failure");
    NSAssert(![self isOnInternalLoggerQueue], @"MUST access ivar directly, NOT via self.* syntax.");
    
    dispatch_queue_t globalLoggingQueue = [DDLog loggingQueue];
    
    dispatch_async(globalLoggingQueue, ^{
        dispatch_async(loggerQueue, block);
    });
}

- (NSTimeInterval)rollingFrequency
{
    __block NSTimeInterval result;
    
    dispatch_block_t block = ^{
        result = rollingFrequency;
    };
    
    // The design of this method is taken from the DDAbstractLogger implementation.
    // For extensive documentation please refer to the DDAbstractLogger implementation.
    
    // Note: The internal implementation should access the rollingFrequency variable directly,
    // This method is designed explicitly for external access.
    //
    // Using "self." syntax to go through this method will cause immediate deadlock.
    // This is the intended result. Fix it by accessing the ivar directly.
    // Great strides have been take to ensure this is safe to do. Plus it's MUCH faster.
    
    NSAssert(![self isOnGlobalLoggingQueue], @"Core architecture requirement failure");
    NSAssert(![self isOnInternalLoggerQueue], @"MUST access ivar directly, NOT via self.* syntax.");
    
    dispatch_queue_t globalLoggingQueue = [DDLog loggingQueue];
    
    dispatch_sync(globalLoggingQueue, ^{
        dispatch_sync(loggerQueue, block);
    });
    
    return result;
}

- (void)setRollingFrequency:(NSTimeInterval)newRollingFrequency
{
    dispatch_block_t block = ^{ @autoreleasepool {
        
        rollingFrequency = newRollingFrequency;
        [self maybeRollLogFileDueToAge];
    }};
    
    // The design of this method is taken from the DDAbstractLogger implementation.
    // For extensive documentation please refer to the DDAbstractLogger implementation.
    
    // Note: The internal implementation should access the rollingFrequency variable directly,
    // This method is designed explicitly for external access.
    //
    // Using "self." syntax to go through this method will cause immediate deadlock.
    // This is the intended result. Fix it by accessing the ivar directly.
    // Great strides have been take to ensure this is safe to do. Plus it's MUCH faster.
    
    NSAssert(![self isOnGlobalLoggingQueue], @"Core architecture requirement failure");
    NSAssert(![self isOnInternalLoggerQueue], @"MUST access ivar directly, NOT via self.* syntax.");
    
    dispatch_queue_t globalLoggingQueue = [DDLog loggingQueue];
    
    dispatch_async(globalLoggingQueue, ^{
        dispatch_async(loggerQueue, block);
    });
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma mark File Rolling
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void)scheduleTimerToRollLogFileDueToAge
{
    if (rollingTimer)
    {
        dispatch_source_cancel(rollingTimer);
        rollingTimer = NULL;
    }
    
    if (currentLogFileInfo == nil || rollingFrequency <= 0.0)
    {
        return;
    }
    
    NSDate *logFileCreationDate = [currentLogFileInfo creationDate];
    
    NSTimeInterval ti = [logFileCreationDate timeIntervalSinceReferenceDate];
    ti += rollingFrequency;
    
    NSDate *logFileRollingDate = [NSDate dateWithTimeIntervalSinceReferenceDate:ti];
    
    NSLogVerbose(@"DDFileLogger: scheduleTimerToRollLogFileDueToAge");
    
    NSLogVerbose(@"DDFileLogger: logFileCreationDate: %@", logFileCreationDate);
    NSLogVerbose(@"DDFileLogger: logFileRollingDate : %@", logFileRollingDate);
    
    rollingTimer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, loggerQueue);
    
    dispatch_source_set_event_handler(rollingTimer, ^{ @autoreleasepool {
        
        [self maybeRollLogFileDueToAge];
        
    }});
    
    #if !OS_OBJECT_USE_OBJC
    dispatch_source_t theRollingTimer = rollingTimer;
    dispatch_source_set_cancel_handler(rollingTimer, ^{
        dispatch_release(theRollingTimer);
    });
    #endif
    
    uint64_t delay = (uint64_t)([logFileRollingDate timeIntervalSinceNow] * NSEC_PER_SEC);
    dispatch_time_t fireTime = dispatch_time(DISPATCH_TIME_NOW, delay);
    
    dispatch_source_set_timer(rollingTimer, fireTime, DISPATCH_TIME_FOREVER, 1.0);
    dispatch_resume(rollingTimer);
}


- (void)rollLogFile
{
    [self rollLogFileWithCompletionBlock:nil];
}

- (void)rollLogFileWithCompletionBlock:(void (^)())completionBlock
{
    // This method is public.
    // We need to execute the rolling on our logging thread/queue.
    
    dispatch_block_t block = ^{ @autoreleasepool {
        
        [self rollLogFileNow];

        if (completionBlock)
        {
            dispatch_async(dispatch_get_main_queue(), ^{
                completionBlock();
            });
        }
    }};
    
    // The design of this method is taken from the DDAbstractLogger implementation.
    // For extensive documentation please refer to the DDAbstractLogger implementation.
    
    if ([self isOnInternalLoggerQueue])
    {
        block();
    }
    else
    {
        dispatch_queue_t globalLoggingQueue = [DDLog loggingQueue];
        NSAssert(![self isOnGlobalLoggingQueue], @"Core architecture requirement failure");
        
        dispatch_async(globalLoggingQueue, ^{
            dispatch_async(loggerQueue, block);
        });
    }
}

- (void)rollLogFileNow
{
    NSLogVerbose(@"DDFileLogger: rollLogFileNow");
    
    
    if (currentLogFileHandle == nil) return;
    
    [currentLogFileHandle synchronizeFile];
    [currentLogFileHandle closeFile];
    currentLogFileHandle = nil;
    
    currentLogFileInfo.isArchived = YES;
    
    if ([logFileManager respondsToSelector:@selector(didRollAndArchiveLogFile:)])
    {
        [logFileManager didRollAndArchiveLogFile:(currentLogFileInfo.filePath)];
    }
    
    currentLogFileInfo = nil;
    
    if (currentLogFileVnode) {
        dispatch_source_cancel(currentLogFileVnode);
        currentLogFileVnode = NULL;
    }

    if (rollingTimer)
    {
        dispatch_source_cancel(rollingTimer);
        rollingTimer = NULL;
    }
}

- (void)maybeRollLogFileDueToAge
{
    if (rollingFrequency > 0.0 && currentLogFileInfo.age >= rollingFrequency)
    {
        NSLogVerbose(@"DDFileLogger: Rolling log file due to age...");
        
        [self rollLogFileNow];
    }
    else
    {
        [self scheduleTimerToRollLogFileDueToAge];
    }
}

- (void)maybeRollLogFileDueToSize
{
    // This method is called from logMessage.
    // Keep it FAST.
    
    // Note: Use direct access to maximumFileSize variable.
    // We specifically wrote our own getter/setter method to allow us to do this (for performance reasons).
    
    if (maximumFileSize > 0)
    {
        unsigned long long fileSize = [currentLogFileHandle offsetInFile];
        
        if (fileSize >= maximumFileSize)
        {
            NSLogVerbose(@"DDFileLogger: Rolling log file due to size (%qu)...", fileSize);
            
            [self rollLogFileNow];
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma mark File Logging
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Returns the log file that should be used.
 * If there is an existing log file that is suitable,
 * within the constraints of maximumFileSize and rollingFrequency, then it is returned.
 * 
 * Otherwise a new file is created and returned.
**/
- (DDLogFileInfo *)currentLogFileInfo
{
    if (currentLogFileInfo == nil)
    {
        NSArray *sortedLogFileInfos = [logFileManager sortedLogFileInfos];
        
        if ([sortedLogFileInfos count] > 0)
        {
            DDLogFileInfo *mostRecentLogFileInfo = [sortedLogFileInfos objectAtIndex:0];
            
            BOOL useExistingLogFile = YES;
            BOOL shouldArchiveMostRecent = NO;
            
            if (mostRecentLogFileInfo.isArchived)
            {
                useExistingLogFile = NO;
                shouldArchiveMostRecent = NO;
            }
            else if (maximumFileSize > 0 && mostRecentLogFileInfo.fileSize >= maximumFileSize)
            {
                useExistingLogFile = NO;
                shouldArchiveMostRecent = YES;
            }
            else if (rollingFrequency > 0.0 && mostRecentLogFileInfo.age >= rollingFrequency)
            {
                useExistingLogFile = NO;
                shouldArchiveMostRecent = YES;
            }


        #if TARGET_OS_IPHONE
            // When creating log file on iOS we're setting NSFileProtectionKey attribute to NSFileProtectionCompleteUnlessOpen.
            // 
            // But in case if app is able to launch from background we need to have an ability to open log file any time we
            // want (even if device is locked). Thats why that attribute have to be changed to
            // NSFileProtectionCompleteUntilFirstUserAuthentication.
            //
            // If previous log was created when app wasn't running in background, but now it is - we archive it and create
            // a new one.

            if (useExistingLogFile && doesAppRunInBackground()) {
                NSString *key = mostRecentLogFileInfo.fileAttributes[NSFileProtectionKey];

                if (! [key isEqualToString:NSFileProtectionCompleteUntilFirstUserAuthentication]) {
                    useExistingLogFile = NO;
                    shouldArchiveMostRecent = YES;
                }
            }
        #endif
            
            if (useExistingLogFile)
            {
                NSLogVerbose(@"DDFileLogger: Resuming logging with file %@", mostRecentLogFileInfo.fileName);
                
                currentLogFileInfo = mostRecentLogFileInfo;
            }
            else
            {
                if (shouldArchiveMostRecent)
                {
                    mostRecentLogFileInfo.isArchived = YES;
                    
                    if ([logFileManager respondsToSelector:@selector(didArchiveLogFile:)])
                    {
                        [logFileManager didArchiveLogFile:(mostRecentLogFileInfo.filePath)];
                    }
                }
            }
        }
        
        if (currentLogFileInfo == nil)
        {
            NSString *currentLogFilePath = [logFileManager createNewLogFile];
            
            currentLogFileInfo = [[DDLogFileInfo alloc] initWithFilePath:currentLogFilePath];
        }
    }
    
    return currentLogFileInfo;
}

- (NSFileHandle *)currentLogFileHandle
{
    if (currentLogFileHandle == nil)
    {
        NSString *logFilePath = [[self currentLogFileInfo] filePath];
        
        currentLogFileHandle = [NSFileHandle fileHandleForWritingAtPath:logFilePath];
        [currentLogFileHandle seekToEndOfFile];
        
        if (currentLogFileHandle)
        {
            [self scheduleTimerToRollLogFileDueToAge];

            // Here we are monitoring the log file. In case if it would be deleted ormoved
            // somewhere we want to roll it and use a new one.
            currentLogFileVnode = dispatch_source_create(
                DISPATCH_SOURCE_TYPE_VNODE,
                [currentLogFileHandle fileDescriptor],
                DISPATCH_VNODE_DELETE | DISPATCH_VNODE_RENAME,
                loggerQueue
            );

            dispatch_source_set_event_handler(currentLogFileVnode, ^{ @autoreleasepool {
                NSLogInfo(@"DDFileLogger: Current logfile was moved. Rolling it and creating a new one");
                [self rollLogFileNow];
            }});

            #if !OS_OBJECT_USE_OBJC
            dispatch_source_t vnode = currentLogFileVnode;
            dispatch_source_set_cancel_handler(currentLogFileVnode, ^{
                dispatch_release(vnode);
            });
            #endif

            dispatch_resume(currentLogFileVnode);
        }
    }
    
    return currentLogFileHandle;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma mark DDLogger Protocol
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int exception_count = 0;
- (void)logMessage:(DDLogMessage *)logMessage
{
    NSString *logMsg = logMessage->logMsg;
    
    if (formatter)
    {
        logMsg = [formatter formatLogMessage:logMessage];
    }
    
    if (logMsg)
    {
        if (![logMsg hasSuffix:@"\n"])
        {
            logMsg = [logMsg stringByAppendingString:@"\n"];
        }
        
        NSData *logData = [logMsg dataUsingEncoding:NSUTF8StringEncoding];

        @try {
            [[self currentLogFileHandle] writeData:logData];

            [self maybeRollLogFileDueToSize];
        }
        @catch (NSException *exception) {
            exception_count++;
            if (exception_count <= 10) {
                NSLogError(@"DDFileLogger.logMessage: %@", exception);
                if (exception_count == 10)
                    NSLogError(@"DDFileLogger.logMessage: Too many exceptions -- will not log any more of them.");
            }
        }
    }
}

- (void)willRemoveLogger
{
    // If you override me be sure to invoke [super willRemoveLogger];
    
    [self rollLogFileNow];
}

- (NSString *)loggerName
{
    return @"cocoa.lumberjack.fileLogger";
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma mark -
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if TARGET_IPHONE_SIMULATOR
  #define XATTR_ARCHIVED_NAME  @"archived"
#else
  #define XATTR_ARCHIVED_NAME  @"lumberjack.log.archived"
#endif

@implementation DDLogFileInfo

@synthesize filePath;

@dynamic fileName;
@dynamic fileAttributes;
@dynamic creationDate;
@dynamic modificationDate;
@dynamic fileSize;
@dynamic age;

@dynamic isArchived;


#pragma mark Lifecycle

+ (instancetype)logFileWithPath:(NSString *)aFilePath
{
    return [[self alloc] initWithFilePath:aFilePath];
}

- (instancetype)initWithFilePath:(NSString *)aFilePath
{
    if ((self = [super init]))
    {
        filePath = [aFilePath copy];
    }
    return self;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma mark Standard Info
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

- (NSDictionary *)fileAttributes
{
    if (fileAttributes == nil)
    {
        fileAttributes = [[NSFileManager defaultManager] attributesOfItemAtPath:filePath error:nil];
    }
    return fileAttributes;
}

- (NSString *)fileName
{
    if (fileName == nil)
    {
        fileName = [filePath lastPathComponent];
    }
    return fileName;
}

- (NSDate *)modificationDate
{
    if (modificationDate == nil)
    {
        modificationDate = [[self fileAttributes] objectForKey:NSFileModificationDate];
    }
    
    return modificationDate;
}

- (NSDate *)creationDate
{
    if (creationDate == nil)
    {
        creationDate = [[self fileAttributes] objectForKey:NSFileCreationDate];
    }
    return creationDate;
}

- (unsigned long long)fileSize
{
    if (fileSize == 0)
    {
        fileSize = [[[self fileAttributes] objectForKey:NSFileSize] unsignedLongLongValue];
    }
    
    return fileSize;
}

- (NSTimeInterval)age
{
    return [[self creationDate] timeIntervalSinceNow] * -1.0;
}

- (NSString *)description
{
    return [@{@"filePath": self.filePath ?: @"",
              @"fileName": self.fileName ?: @"",
              @"fileAttributes": self.fileAttributes ?: @"",
              @"creationDate": self.creationDate ?: @"",
              @"modificationDate": self.modificationDate ?: @"",
              @"fileSize": @(self.fileSize),
              @"age": @(self.age),
              @"isArchived": @(self.isArchived)} description];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma mark Archiving
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

- (BOOL)isArchived
{
    
#if TARGET_IPHONE_SIMULATOR
    
    // Extended attributes don't work properly on the simulator.
    // So we have to use a less attractive alternative.
    // See full explanation in the header file.
    
    return [self hasExtensionAttributeWithName:XATTR_ARCHIVED_NAME];
    
#else
    
    return [self hasExtendedAttributeWithName:XATTR_ARCHIVED_NAME];
    
#endif
}

- (void)setIsArchived:(BOOL)flag
{
    
#if TARGET_IPHONE_SIMULATOR
    
    // Extended attributes don't work properly on the simulator.
    // So we have to use a less attractive alternative.
    // See full explanation in the header file.
    
    if (flag)
        [self addExtensionAttributeWithName:XATTR_ARCHIVED_NAME];
    else
        [self removeExtensionAttributeWithName:XATTR_ARCHIVED_NAME];
    
#else
    
    if (flag)
        [self addExtendedAttributeWithName:XATTR_ARCHIVED_NAME];
    else
        [self removeExtendedAttributeWithName:XATTR_ARCHIVED_NAME];
    
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma mark Changes
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void)reset
{
    fileName = nil;
    fileAttributes = nil;
    creationDate = nil;
    modificationDate = nil;
}

- (void)renameFile:(NSString *)newFileName
{
    // This method is only used on the iPhone simulator, where normal extended attributes are broken.
    // See full explanation in the header file.
    
    if (![newFileName isEqualToString:[self fileName]])
    {
        NSString *fileDir = [filePath stringByDeletingLastPathComponent];
        
        NSString *newFilePath = [fileDir stringByAppendingPathComponent:newFileName];
        
        NSLogVerbose(@"DDLogFileInfo: Renaming file: '%@' -> '%@'", self.fileName, newFileName);
        
        NSError *error = nil;
        if (![[NSFileManager defaultManager] moveItemAtPath:filePath toPath:newFilePath error:&error])
        {
            NSLogError(@"DDLogFileInfo: Error renaming file (%@): %@", self.fileName, error);
        }
        
        filePath = newFilePath;
        [self reset];
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma mark Attribute Management
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if TARGET_IPHONE_SIMULATOR

// Extended attributes don't work properly on the simulator.
// So we have to use a less attractive alternative.
// See full explanation in the header file.

- (BOOL)hasExtensionAttributeWithName:(NSString *)attrName
{
    // This method is only used on the iPhone simulator, where normal extended attributes are broken.
    // See full explanation in the header file.
    
    // Split the file name into components.
    // 
    // log-ABC123.archived.uploaded.txt
    // 
    // 0. log-ABC123
    // 1. archived
    // 2. uploaded
    // 3. txt
    // 
    // So we want to search for the attrName in the components (ignoring the first and last array indexes).
    
    NSArray *components = [[self fileName] componentsSeparatedByString:@"."];
    
    // Watch out for file names without an extension
    
    NSUInteger count = [components count];
    NSUInteger max = (count >= 2) ? count-1 : count;
    
    NSUInteger i;
    for (i = 1; i < max; i++)
    {
        NSString *attr = [components objectAtIndex:i];
        
        if ([attrName isEqualToString:attr])
        {
            return YES;
        }
    }
    
    return NO;
}

- (void)addExtensionAttributeWithName:(NSString *)attrName
{
    // This method is only used on the iPhone simulator, where normal extended attributes are broken.
    // See full explanation in the header file.
    
    if ([attrName length] == 0) return;
    
    // Example:
    // attrName = "archived"
    // 
    // "log-ABC123.txt" -> "log-ABC123.archived.txt"
    
    NSArray *components = [[self fileName] componentsSeparatedByString:@"."];
    
    NSUInteger count = [components count];
    
    NSUInteger estimatedNewLength = [[self fileName] length] + [attrName length] + 1;
    NSMutableString *newFileName = [NSMutableString stringWithCapacity:estimatedNewLength];
    
    if (count > 0)
    {
        [newFileName appendString:[components objectAtIndex:0]];
    }
    
    NSString *lastExt = @"";
    
    NSUInteger i;
    for (i = 1; i < count; i++)
    {
        NSString *attr = [components objectAtIndex:i];
        if ([attr length] == 0)
        {
            continue;
        }
        
        if ([attrName isEqualToString:attr])
        {
            // Extension attribute already exists in file name
            return;
        }
        
        if ([lastExt length] > 0)
        {
            [newFileName appendFormat:@".%@", lastExt];
        }
        
        lastExt = attr;
    }
    
    [newFileName appendFormat:@".%@", attrName];
    
    if ([lastExt length] > 0)
    {
        [newFileName appendFormat:@".%@", lastExt];
    }
    
    [self renameFile:newFileName];
}

- (void)removeExtensionAttributeWithName:(NSString *)attrName
{
    // This method is only used on the iPhone simulator, where normal extended attributes are broken.
    // See full explanation in the header file.
    
    if ([attrName length] == 0) return;
    
    // Example:
    // attrName = "archived"
    // 
    // "log-ABC123.archived.txt" -> "log-ABC123.txt"
    
    NSArray *components = [[self fileName] componentsSeparatedByString:@"."];
    
    NSUInteger count = [components count];
    
    NSUInteger estimatedNewLength = [[self fileName] length];
    NSMutableString *newFileName = [NSMutableString stringWithCapacity:estimatedNewLength];
    
    if (count > 0)
    {
        [newFileName appendString:[components objectAtIndex:0]];
    }
    
    BOOL found = NO;
    
    NSUInteger i;
    for (i = 1; i < count; i++)
    {
        NSString *attr = [components objectAtIndex:i];
        
        if ([attrName isEqualToString:attr])
        {
            found = YES;
        }
        else
        {
            [newFileName appendFormat:@".%@", attr];
        }
    }
    
    if (found)
    {
        [self renameFile:newFileName];
    }
}

#else

- (BOOL)hasExtendedAttributeWithName:(NSString *)attrName
{
    const char *path = [filePath UTF8String];
    const char *name = [attrName UTF8String];
    
    ssize_t result = getxattr(path, name, NULL, 0, 0, 0);
    
    return (result >= 0);
}

- (void)addExtendedAttributeWithName:(NSString *)attrName
{
    const char *path = [filePath UTF8String];
    const char *name = [attrName UTF8String];
    
    int result = setxattr(path, name, NULL, 0, 0, 0);
    
    if (result < 0)
    {
        NSLogError(@"DDLogFileInfo: setxattr(%@, %@): error = %i", attrName, self.fileName, result);
    }
}

- (void)removeExtendedAttributeWithName:(NSString *)attrName
{
    const char *path = [filePath UTF8String];
    const char *name = [attrName UTF8String];
    
    int result = removexattr(path, name, 0);
    
    if (result < 0 && errno != ENOATTR)
    {
        NSLogError(@"DDLogFileInfo: removexattr(%@, %@): error = %i", attrName, self.fileName, result);
    }
}

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma mark Comparisons
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

- (BOOL)isEqual:(id)object
{
    if ([object isKindOfClass:[self class]])
    {
        DDLogFileInfo *another = (DDLogFileInfo *)object;
        
        return [filePath isEqualToString:[another filePath]];
    }
    
    return NO;
}

- (NSComparisonResult)reverseCompareByCreationDate:(DDLogFileInfo *)another
{
    NSDate *us = [self creationDate];
    NSDate *them = [another creationDate];
    
    NSComparisonResult result = [us compare:them];
    
    if (result == NSOrderedAscending)
        return NSOrderedDescending;
    
    if (result == NSOrderedDescending)
        return NSOrderedAscending;
    
    return NSOrderedSame;
}

- (NSComparisonResult)reverseCompareByModificationDate:(DDLogFileInfo *)another
{
    NSDate *us = [self modificationDate];
    NSDate *them = [another modificationDate];
    
    NSComparisonResult result = [us compare:them];
    
    if (result == NSOrderedAscending)
        return NSOrderedDescending;
    
    if (result == NSOrderedDescending)
        return NSOrderedAscending;
    
    return NSOrderedSame;
}

@end

#if TARGET_OS_IPHONE
/**
 * When creating log file on iOS we're setting NSFileProtectionKey attribute to NSFileProtectionCompleteUnlessOpen.
 *
 * But in case if app is able to launch from background we need to have an ability to open log file any time we
 * want (even if device is locked). Thats why that attribute have to be changed to
 * NSFileProtectionCompleteUntilFirstUserAuthentication.
 */
BOOL doesAppRunInBackground()
{
    BOOL answer = NO;

    NSArray *backgroundModes = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"UIBackgroundModes"];

    for (NSString *mode in backgroundModes) {
        if (mode.length > 0) {
            answer = YES;
            break;
        }
    }

    return answer;
}
#endif

