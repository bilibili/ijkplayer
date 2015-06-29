//
//  DDLog+LOGV.h
//  Lumberjack
//
//  Created by Mike Pontillo on 11/20/12.
//
//

#ifndef Lumberjack_DDLog_LOGV_h
#define Lumberjack_DDLog_LOGV_h

#import "DDLog.h"


#define LOGV_MACRO(isAsynchronous, lvl, flg, ctx, atag, fnct, frmt, avalist) \
  [DDLog log:isAsynchronous                                                  \
       level:lvl                                                             \
        flag:flg                                                             \
     context:ctx                                                             \
        file:__FILE__                                                        \
    function:fnct                                                            \
        line:__LINE__                                                        \
         tag:atag                                                            \
      format:frmt                                                            \
        args:avalist]

#define LOGV_OBJC_MACRO(async, lvl, flg, ctx, frmt, avalist) \
             LOGV_MACRO(async, lvl, flg, ctx, nil, sel_getName(_cmd), frmt, avalist)

#define LOGV_C_MACRO(async, lvl, flg, ctx, frmt, avalist) \
          LOGV_MACRO(async, lvl, flg, ctx, nil, __FUNCTION__, frmt, avalist)



#define  SYNC_LOGV_OBJC_MACRO(lvl, flg, ctx, frmt, avalist) \
              LOGV_OBJC_MACRO( NO, lvl, flg, ctx, frmt, avalist)

#define ASYNC_LOGV_OBJC_MACRO(lvl, flg, ctx, frmt, avalist) \
              LOGV_OBJC_MACRO(YES, lvl, flg, ctx, frmt, avalist)

#define  SYNC_LOGV_C_MACRO(lvl, flg, ctx, frmt, avalist) \
              LOGV_C_MACRO( NO, lvl, flg, ctx, frmt, avalist)

#define ASYNC_LOGV_C_MACRO(lvl, flg, ctx, frmt, avalist) \
              LOGV_C_MACRO(YES, lvl, flg, ctx, frmt, avalist)



#define LOGV_MAYBE(async, lvl, flg, ctx, fnct, frmt, avalist) \
  do { if(lvl & flg) LOGV_MACRO(async, lvl, flg, ctx, nil, fnct, frmt, avalist); } while(0)

#define LOGV_OBJC_MAYBE(async, lvl, flg, ctx, frmt, avalist) \
             LOGV_MAYBE(async, lvl, flg, ctx, sel_getName(_cmd), frmt, avalist)

#define LOGV_C_MAYBE(async, lvl, flg, ctx, frmt, avalist) \
          LOGV_MAYBE(async, lvl, flg, ctx, __FUNCTION__, frmt, avalist)

#define  SYNC_LOGV_OBJC_MAYBE(lvl, flg, ctx, frmt, avalist) \
              LOGV_OBJC_MAYBE( NO, lvl, flg, ctx, frmt, avalist)

#define ASYNC_LOGV_OBJC_MAYBE(lvl, flg, ctx, frmt, avalist) \
              LOGV_OBJC_MAYBE(YES, lvl, flg, ctx, frmt, avalist)

#define  SYNC_LOGV_C_MAYBE(lvl, flg, ctx, frmt, avalist) \
              LOGV_C_MAYBE( NO, lvl, flg, ctx, frmt, avalist)

#define ASYNC_LOGV_C_MAYBE(lvl, flg, ctx, frmt, avalist) \
              LOGV_C_MAYBE(YES, lvl, flg, ctx, frmt, avalist)



#define LOGV_OBJC_TAG_MACRO(async, lvl, flg, ctx, tag, frmt, avalist) \
                 LOGV_MACRO(async, lvl, flg, ctx, tag, sel_getName(_cmd), frmt, avalist)

#define LOGV_C_TAG_MACRO(async, lvl, flg, ctx, tag, frmt, avalist) \
              LOGV_MACRO(async, lvl, flg, ctx, tag, __FUNCTION__, frmt, avalist)

#define LOGV_TAG_MAYBE(async, lvl, flg, ctx, tag, fnct, frmt, avalist) \
  do { if(lvl & flg) LOGV_MACRO(async, lvl, flg, ctx, tag, fnct, frmt, avalist); } while(0)

#define LOGV_OBJC_TAG_MAYBE(async, lvl, flg, ctx, tag, frmt, avalist) \
             LOGV_TAG_MAYBE(async, lvl, flg, ctx, tag, sel_getName(_cmd), frmt, avalist)

#define LOGV_C_TAG_MAYBE(async, lvl, flg, ctx, tag, frmt, avalist) \
          LOGV_TAG_MAYBE(async, lvl, flg, ctx, tag, __FUNCTION__, frmt, avalist)



#define DDLogvError(frmt, avalist)   LOGV_OBJC_MAYBE(LOG_ASYNC_ERROR,   ddLogLevel, LOG_FLAG_ERROR,   0, frmt, avalist)
#define DDLogvWarn(frmt, avalist)    LOGV_OBJC_MAYBE(LOG_ASYNC_WARN,    ddLogLevel, LOG_FLAG_WARN,    0, frmt, avalist)
#define DDLogvInfo(frmt, avalist)    LOGV_OBJC_MAYBE(LOG_ASYNC_INFO,    ddLogLevel, LOG_FLAG_INFO,    0, frmt, avalist)
#define DDLogvVerbose(frmt, avalist) LOGV_OBJC_MAYBE(LOG_ASYNC_VERBOSE, ddLogLevel, LOG_FLAG_VERBOSE, 0, frmt, avalist)

#define DDLogvCError(frmt, avalist)   LOGV_C_MAYBE(LOG_ASYNC_ERROR,   ddLogLevel, LOG_FLAG_ERROR,   0, frmt, avalist)
#define DDLogvCWarn(frmt, avalist)    LOGV_C_MAYBE(LOG_ASYNC_WARN,    ddLogLevel, LOG_FLAG_WARN,    0, frmt, avalist)
#define DDLogvCInfo(frmt, avalist)    LOGV_C_MAYBE(LOG_ASYNC_INFO,    ddLogLevel, LOG_FLAG_INFO,    0, frmt, avalist)
#define DDLogvCVerbose(frmt, avalist) LOGV_C_MAYBE(LOG_ASYNC_VERBOSE, ddLogLevel, LOG_FLAG_VERBOSE, 0, frmt, avalist)

#endif
