//
//  IJKLog.h
//  IJKMediaPlayer
//
//  Created by Ken Sun on 2016/9/12.
//  Copyright © 2016年 bilibili. All rights reserved.
//

#ifndef IJKLog_h
#define IJKLog_h

#ifdef DEBUG
#define IJKLog NSLog
#else
#define IJKLog
#endif

#endif /* IJKLog_h */
