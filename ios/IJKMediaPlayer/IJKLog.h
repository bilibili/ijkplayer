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

void
print_bytes(void   *start,
            size_t  length)
{
    uint8_t *base = NULL;
    size_t   idx = 0;
    
    if (!start || length <= 0)
        return;
    
    base = (uint8_t *)(start);
    for (idx = 0; idx < length; idx++)
        printf("%02X%s", base[idx] & 0xFF, (idx + 1) % 16 == 0 ? "\n" : " ");
    printf("\n");
}

#endif /* IJKLog_h */
