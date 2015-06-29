#import <Foundation/Foundation.h>
#import "DDLog.h"

@class DDLogFileInfo;

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
 * This class provides a logger to write log statements to a file.
**/


// Default configuration and safety/sanity values.
// 
// maximumFileSize         -> DEFAULT_LOG_MAX_FILE_SIZE
// rollingFrequency        -> DEFAULT_LOG_ROLLING_FREQUENCY
// maximumNumberOfLogFiles -> DEFAULT_LOG_MAX_NUM_LOG_FILES
// 
// You should carefully consider the proper configuration values for your application.

#define DEFAULT_LOG_MAX_FILE_SIZE     (1024 * 1024)   //  1 MB
#define DEFAULT_LOG_ROLLING_FREQUENCY (60 * 60 * 24)  // 24 Hours
#define DEFAULT_LOG_MAX_NUM_LOG_FILES (5)             //  5 Files


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma mark -
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// The LogFileManager protocol is designed to allow you to control all aspects of your log files.
// 
// The primary purpose of this is to allow you to do something with the log files after they have been rolled.
// Perhaps you want to compress them to save disk space.
// Perhaps you want to upload them to an FTP server.
// Perhaps you want to run some analytics on the file.
// 
// A default LogFileManager is, of course, provided.
// The default LogFileManager simply deletes old log files according to the maximumNumberOfLogFiles property.
// 
// This protocol provides various methods to fetch the list of log files.
// 
// There are two variants: sorted and unsorted.
// If sorting is not necessary, the unsorted variant is obviously faster.
// The sorted variant will return an array sorted by when the log files were created,
// with the most recently created log file at index 0, and the oldest log file at the end of the array.
// 
// You can fetch only the log file paths (full path including name), log file names (name only),
// or an array of DDLogFileInfo objects.
// The DDLogFileInfo class is documented below, and provides a handy wrapper that
// gives you easy access to various file attributes such as the creation date or the file size.

@protocol DDLogFileManager <NSObject>
@required

// Public properties

/**
 * The maximum number of archived log files to keep on disk.
 * For example, if this property is set to 3,
 * then the LogFileManager will only keep 3 archived log files (plus the current active log file) on disk.
 * Once the active log file is rolled/archived, then the oldest of the existing 3 rolled/archived log files is deleted.
 * 
 * You may optionally disable deleting old/rolled/archived log files by setting this property to zero.
**/
@property (readwrite, assign) NSUInteger maximumNumberOfLogFiles;

// Public methods

- (NSString *)logsDirectory;

- (NSArray *)unsortedLogFilePaths;
- (NSArray *)unsortedLogFileNames;
- (NSArray *)unsortedLogFileInfos;

- (NSArray *)sortedLogFilePaths;
- (NSArray *)sortedLogFileNames;
- (NSArray *)sortedLogFileInfos;

// Private methods (only to be used by DDFileLogger)

- (NSString *)createNewLogFile;

@optional

// Notifications from DDFileLogger

- (void)didArchiveLogFile:(NSString *)logFilePath;
- (void)didRollAndArchiveLogFile:(NSString *)logFilePath;

@end

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma mark -
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Default log file manager.
 * 
 * All log files are placed inside the logsDirectory.
 * If a specific logsDirectory isn't specified, the default directory is used.
 * On Mac, this is in ~/Library/Logs/<Application Name>.
 * On iPhone, this is in ~/Library/Caches/Logs.
 * 
 * Log files are named "<app name> <date> <time>.log"
 * Example: MobileSafari 2013-12-03 17-14.log
 * 
 * Archived log files are automatically deleted according to the maximumNumberOfLogFiles property.
**/
@interface DDLogFileManagerDefault : NSObject <DDLogFileManager>
{
    NSUInteger maximumNumberOfLogFiles;
    NSString *_logsDirectory;
}

- (id)init;
- (instancetype)initWithLogsDirectory:(NSString *)logsDirectory;

/* Inherited from DDLogFileManager protocol:

@property (readwrite, assign) NSUInteger maximumNumberOfLogFiles;

- (NSString *)logsDirectory;

- (NSArray *)unsortedLogFilePaths;
- (NSArray *)unsortedLogFileNames;
- (NSArray *)unsortedLogFileInfos;

- (NSArray *)sortedLogFilePaths;
- (NSArray *)sortedLogFileNames;
- (NSArray *)sortedLogFileInfos;

*/

@end

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma mark -
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Most users will want file log messages to be prepended with the date and time.
 * Rather than forcing the majority of users to write their own formatter,
 * we will supply a logical default formatter.
 * Users can easily replace this formatter with their own by invoking the setLogFormatter method.
 * It can also be removed by calling setLogFormatter, and passing a nil parameter.
 * 
 * In addition to the convenience of having a logical default formatter,
 * it will also provide a template that makes it easy for developers to copy and change.
**/
@interface DDLogFileFormatterDefault : NSObject <DDLogFormatter>
{
    NSDateFormatter *dateFormatter;
}

- (id)init;
- (instancetype)initWithDateFormatter:(NSDateFormatter *)dateFormatter;

@end

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma mark -
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

@interface DDFileLogger : DDAbstractLogger <DDLogger>
{
    __strong id <DDLogFileManager> logFileManager;
    
    DDLogFileInfo *currentLogFileInfo;
    NSFileHandle *currentLogFileHandle;
    
    dispatch_source_t currentLogFileVnode;
    dispatch_source_t rollingTimer;
    
    unsigned long long maximumFileSize;
    NSTimeInterval rollingFrequency;
}

- (id)init;
- (instancetype)initWithLogFileManager:(id <DDLogFileManager>)logFileManager;

/**
 * Log File Rolling:
 * 
 * maximumFileSize:
 *   The approximate maximum size to allow log files to grow.
 *   If a log file is larger than this value after a log statement is appended,
 *   then the log file is rolled.
 * 
 * rollingFrequency
 *   How often to roll the log file.
 *   The frequency is given as an NSTimeInterval, which is a double that specifies the interval in seconds.
 *   Once the log file gets to be this old, it is rolled.
 * 
 * Both the maximumFileSize and the rollingFrequency are used to manage rolling.
 * Whichever occurs first will cause the log file to be rolled.
 * 
 * For example:
 * The rollingFrequency is 24 hours,
 * but the log file surpasses the maximumFileSize after only 20 hours.
 * The log file will be rolled at that 20 hour mark.
 * A new log file will be created, and the 24 hour timer will be restarted.
 * 
 * You may optionally disable rolling due to filesize by setting maximumFileSize to zero.
 * If you do so, rolling is based solely on rollingFrequency.
 * 
 * You may optionally disable rolling due to time by setting rollingFrequency to zero (or any non-positive number).
 * If you do so, rolling is based solely on maximumFileSize.
 * 
 * If you disable both maximumFileSize and rollingFrequency, then the log file won't ever be rolled.
 * This is strongly discouraged.
**/
@property (readwrite, assign) unsigned long long maximumFileSize;
@property (readwrite, assign) NSTimeInterval rollingFrequency;

/**
 * The DDLogFileManager instance can be used to retrieve the list of log files,
 * and configure the maximum number of archived log files to keep.
 * 
 * @see DDLogFileManager.maximumNumberOfLogFiles
**/
@property (strong, nonatomic, readonly) id <DDLogFileManager> logFileManager;


// You can optionally force the current log file to be rolled with this method.
// CompletionBlock will be called on main queue.

- (void)rollLogFileWithCompletionBlock:(void (^)())completionBlock;

// Method is deprecated. Use rollLogFileWithCompletionBlock: method instead.

- (void)rollLogFile __attribute((deprecated));

// Inherited from DDAbstractLogger

// - (id <DDLogFormatter>)logFormatter;
// - (void)setLogFormatter:(id <DDLogFormatter>)formatter;

@end

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma mark -
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * DDLogFileInfo is a simple class that provides access to various file attributes.
 * It provides good performance as it only fetches the information if requested,
 * and it caches the information to prevent duplicate fetches.
 * 
 * It was designed to provide quick snapshots of the current state of log files,
 * and to help sort log files in an array.
 * 
 * This class does not monitor the files, or update it's cached attribute values if the file changes on disk.
 * This is not what the class was designed for.
 * 
 * If you absolutely must get updated values,
 * you can invoke the reset method which will clear the cache.
**/
@interface DDLogFileInfo : NSObject
{
    __strong NSString *filePath;
    __strong NSString *fileName;
    
    __strong NSDictionary *fileAttributes;
    
    __strong NSDate *creationDate;
    __strong NSDate *modificationDate;
    
    unsigned long long fileSize;
}

@property (strong, nonatomic, readonly) NSString *filePath;
@property (strong, nonatomic, readonly) NSString *fileName;

@property (strong, nonatomic, readonly) NSDictionary *fileAttributes;

@property (strong, nonatomic, readonly) NSDate *creationDate;
@property (strong, nonatomic, readonly) NSDate *modificationDate;

@property (nonatomic, readonly) unsigned long long fileSize;

@property (nonatomic, readonly) NSTimeInterval age;

@property (nonatomic, readwrite) BOOL isArchived;

+ (instancetype)logFileWithPath:(NSString *)filePath;

- (instancetype)initWithFilePath:(NSString *)filePath;

- (void)reset;
- (void)renameFile:(NSString *)newFileName;

#if TARGET_IPHONE_SIMULATOR

// So here's the situation.
// Extended attributes are perfect for what we're trying to do here (marking files as archived).
// This is exactly what extended attributes were designed for.
// 
// But Apple screws us over on the simulator.
// Everytime you build-and-go, they copy the application into a new folder on the hard drive,
// and as part of the process they strip extended attributes from our log files.
// Normally, a copy of a file preserves extended attributes.
// So obviously Apple has gone to great lengths to piss us off.
// 
// Thus we use a slightly different tactic for marking log files as archived in the simulator.
// That way it "just works" and there's no confusion when testing.
// 
// The difference in method names is indicative of the difference in functionality.
// On the simulator we add an attribute by appending a filename extension.
// 
// For example:
// log-ABC123.txt -> log-ABC123.archived.txt

- (BOOL)hasExtensionAttributeWithName:(NSString *)attrName;

- (void)addExtensionAttributeWithName:(NSString *)attrName;
- (void)removeExtensionAttributeWithName:(NSString *)attrName;

#else

// Normal use of extended attributes used everywhere else,
// such as on Macs and on iPhone devices.

- (BOOL)hasExtendedAttributeWithName:(NSString *)attrName;

- (void)addExtendedAttributeWithName:(NSString *)attrName;
- (void)removeExtendedAttributeWithName:(NSString *)attrName;

#endif

- (NSComparisonResult)reverseCompareByCreationDate:(DDLogFileInfo *)another;
- (NSComparisonResult)reverseCompareByModificationDate:(DDLogFileInfo *)another;

@end
