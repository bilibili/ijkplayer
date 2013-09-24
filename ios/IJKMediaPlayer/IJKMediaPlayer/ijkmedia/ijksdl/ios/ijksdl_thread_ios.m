//
//  ijksdl_thread_ios.m
//  IJKMediaPlayer
//
//  Created by ZhangRui on 13-9-24.
//  Copyright (c) 2013年 bilibili. All rights reserved.
//

#import "ijksdl_thread_ios.h"
#include "ijksdl/ijksdl_thread.h"

static void *SDL_RunThread(void *data)
{
    @autoreleasepool {
        SDL_Thread *thread = data;
        thread->retval = thread->func(thread->data);
        return NULL;
    }
}

SDL_Thread *SDL_CreateThreadEx(SDL_Thread *thread, int (*fn)(void *), void *data, const char *name)
{
    thread->func = fn;
    thread->data = data;
    strlcpy(thread->name, name, sizeof(thread->name) - 1);
    int retval = pthread_create(&thread->id, NULL, SDL_RunThread, thread);
    if (retval)
        return NULL;

    return thread;
}