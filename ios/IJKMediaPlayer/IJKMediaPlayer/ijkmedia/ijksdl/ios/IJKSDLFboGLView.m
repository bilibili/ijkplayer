/*
 * IJKSDLFboGLView.m
 *
 * Copyright (c) 2013 Bilibili
 * Copyright (c) 2019 Befovy <befovy@gmail.com>
 *
 * based on https://github.com/kolyvan/kxmovie
 *
 * This file is part of ijkPlayer.
 *
 * ijkPlayer is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * ijkPlayer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with ijkPlayer; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#import "IJKSDLFboGLView.h"
#import "IJKSDLGLView.h"
#include "ijksdl/ijksdl_timer.h"
#include "ijksdl/ios/ijksdl_ios.h"
#include "ijksdl/ijksdl_gles2.h"

@implementation IJKSDLFboGLView {
    NSRecursiveLock *_glActiveLock;
    BOOL _glActivePaused;
    
    EAGLContext     *_context;
    GLuint          _framebuffer;
    
    IJK_GLES2_Renderer *_renderer;
    int                 _rendererGravity;
    
    int             _tryLockErrorCount;
    BOOL            _didSetupGL;
    BOOL            _didStopGL;
    NSMutableArray *_registeredNotifications;
    
    IJKSDLGLViewApplicationState _applicationState;
    
    CVOpenGLESTextureCacheRef _textureCache;
    CVOpenGLESTextureRef _texture;
    CVPixelBufferRef _target;
    CGSize _renderSize;
    CGSize _fboSize;

    id<IJKCVPBViewProtocol> _cvPBView;
}

@synthesize fps = _fps;
@synthesize isThirdGLView = _isThirdGLView;
@synthesize scaleFactor = _scaleFactor;

- (instancetype)initWithIJKCVPBViewProtocol:(id<IJKCVPBViewProtocol>) cvPBView;
{
    self = [super init];
    if (self) {
        _didSetupGL = NO;
        _tryLockErrorCount = 0;
        _isThirdGLView = false;
        _glActiveLock = [[NSRecursiveLock alloc] init];
        _registeredNotifications = [[NSMutableArray alloc] init];
        [self registerApplicationObservers];
        
        _fboSize = CGSizeMake(0, 0);
        _renderSize = CGSizeMake(720, 720);
        _cvPBView = cvPBView;
        
        if ([self isApplicationActive] == YES)
            [self setupGLOnce];
    }
    return self;
}

- (BOOL)createTextureCache:(EAGLContext *)context
{
    CVReturn err = noErr;
    if (_textureCache == nil) {
        err = CVOpenGLESTextureCacheCreate(kCFAllocatorDefault, NULL, _context, NULL, &_textureCache);
        ALOGD("%s ret %d", __func__, (int)err);
    }
    if (err != noErr) {
        ALOGE("Error at CVOpenGLESTextureCacheCreate %d", err);
        return NO;
    }
    return YES;
}

- (BOOL)createBfo
{
    if (_framebuffer) {
        glDeleteFramebuffers(1, &_framebuffer);
        _framebuffer = 0;
    }
    
    glGenFramebuffers(1, &_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
    ALOGD("%s glGenFramebuffers", __func__);
    
    CFDictionaryRef empty; // empty value for attr value.
    CFMutableDictionaryRef attrs;
    empty = CFDictionaryCreate(kCFAllocatorDefault, // our empty IOSurface properties dictionary
                               NULL,
                               NULL,
                               0,
                               &kCFTypeDictionaryKeyCallBacks,
                               &kCFTypeDictionaryValueCallBacks);
    attrs = CFDictionaryCreateMutable(kCFAllocatorDefault, 1,
                                      &kCFTypeDictionaryKeyCallBacks,
                                      &kCFTypeDictionaryValueCallBacks);
    
    CFDictionarySetValue(attrs, kCVPixelBufferIOSurfacePropertiesKey, empty);
    
    if (_target) {
        CFRelease(_target);
        _target = nil;
    }

    CVPixelBufferCreate(kCFAllocatorDefault, _fboSize.width, _fboSize.height,
                        kCVPixelFormatType_32BGRA,
                        attrs,
                        &_target);
    
    CFRelease(attrs);
    CFRelease(empty);

    ALOGD("CVPixelBufferCreate %f %f\n", _fboSize.width, _fboSize.height);
    if (_texture) {
        CFRelease(_texture);
    }

    CVOpenGLESTextureCacheCreateTextureFromImage (kCFAllocatorDefault,
                                                  _textureCache,
                                                  _target,
                                                  NULL, // texture attributes
                                                  GL_TEXTURE_2D,
                                                  GL_RGBA, // opengl format
                                                  _fboSize.width,
                                                  _fboSize.height,
                                                  GL_BGRA, // native iOS format
                                                  GL_UNSIGNED_BYTE,
                                                  0,
                                                  &_texture);
    

    
    glBindTexture(CVOpenGLESTextureGetTarget(_texture), CVOpenGLESTextureGetName(_texture));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
    glTexImage2D(GL_TEXTURE_2D,
                 0, GL_RGBA, _fboSize.width, _fboSize.height,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    
    glGenFramebuffers(1, &_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, CVOpenGLESTextureGetName(_texture), 0);
    
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        NSLog(@"failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
        return  NO;
    }
    return YES;
}

- (BOOL)setupEAGLContext:(EAGLContext *)context
{
    if (_renderSize.height <= 0 || _renderSize.width <= 0)
        return NO;
    
    if (![self createTextureCache:context])
        return NO;
    
    BOOL ret = YES;
    if (_renderSize.height != _fboSize.height || _renderSize.width != _fboSize.width) {
        _fboSize = _renderSize;
        ret = [self createBfo];
    }
    return ret;
}

- (BOOL)setupGL
{
    if (_didSetupGL)
        return YES;
    _scaleFactor = 1.0f;
    _context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    if (_context == nil) {
        NSLog(@"failed to setup EAGLContext\n");
        return NO;
    }

    EAGLContext *prevContext = [EAGLContext currentContext];
    [EAGLContext setCurrentContext:_context];
    
    _didSetupGL = NO;
    if ([self setupEAGLContext:_context]) {
        NSLog(@"OK setup GL\n");
        _didSetupGL = YES;
    }

    [EAGLContext setCurrentContext:prevContext];
    return _didSetupGL;
}

- (BOOL)setupGLOnce
{
    if (_didSetupGL)
        return YES;
    
    if (![self tryLockGLActive])
        return NO;
    
    BOOL didSetupGL = [self setupGL];
    [self unlockGLActive];
    return didSetupGL;
}

- (BOOL)isApplicationActive
{
    switch (_applicationState) {
        case IJKSDLGLViewApplicationForegroundState:
            return YES;
        case IJKSDLGLViewApplicationBackgroundState:
            return NO;
        default: {
            UIApplicationState appState = [UIApplication sharedApplication].applicationState;
            switch (appState) {
                case UIApplicationStateActive:
                    _applicationState = IJKSDLGLViewApplicationForegroundState;
                    return YES;
                case UIApplicationStateInactive:
                case UIApplicationStateBackground:
                default:
                    _applicationState = IJKSDLGLViewApplicationBackgroundState;
                    return NO;
            }
        }
    }
}

- (void)dealloc
{
    [self lockGLActive];
    
    _didStopGL = YES;
    
    EAGLContext *prevContext = [EAGLContext currentContext];
    [EAGLContext setCurrentContext:_context];
    
    IJK_GLES2_Renderer_reset(_renderer);
    IJK_GLES2_Renderer_freeP(&_renderer);
    
    if (_framebuffer) {
        glDeleteFramebuffers(1, &_framebuffer);
        _framebuffer = 0;
    }
    
    _cvPBView = nil;
    
    if (_target) {
        CFRelease(_target);
        _target = nil;
    }
    
    if (_textureCache) {
        CFRelease(_textureCache);
        _textureCache = nil;
    }
    
    if (_texture) {
        CFRelease(_texture);
        _texture = nil;
    }
    
    glFinish();
    
    [EAGLContext setCurrentContext:prevContext];
    
    _context = nil;
    
    [self unregisterApplicationObservers];
    
    [self unlockGLActive];
}



- (void) setFitmode:(int)x
{
    
}


- (BOOL)setupRenderer: (SDL_VoutOverlay *) overlay
{
    if (overlay == nil)
        return _renderer != nil;
    
    if (!IJK_GLES2_Renderer_isValid(_renderer) ||
        !IJK_GLES2_Renderer_isFormat(_renderer, overlay->format)) {
        
        IJK_GLES2_Renderer_reset(_renderer);
        IJK_GLES2_Renderer_freeP(&_renderer);
        
        _renderer = IJK_GLES2_Renderer_create(overlay);
        if (!IJK_GLES2_Renderer_isValid(_renderer))
            return NO;
        
        if (!IJK_GLES2_Renderer_use(_renderer))
            return NO;
        
        IJK_GLES2_Renderer_setGravity(_renderer, _rendererGravity, _renderSize.width, _renderSize.height);
    }
    
    return YES;
}

- (void) display: (SDL_VoutOverlay *) overlay
{
    if ([self isApplicationActive] == NO)
        return;
    
    if (_didSetupGL == NO)
        return;
    
    if (![self tryLockGLActive]) {
        if (0 == (_tryLockErrorCount % 100)) {
            NSLog(@"IJKSDLGLView:display: unable to tryLock GL active: %d\n", _tryLockErrorCount);
        }
        _tryLockErrorCount++;
        return;
    }
    _tryLockErrorCount = 0;
    if (_context && !_didStopGL) {
        EAGLContext *prevContext = [EAGLContext currentContext];
        [EAGLContext setCurrentContext:_context];
        [self displayInternal:overlay];
        [EAGLContext setCurrentContext:prevContext];
    }
    
    [self unlockGLActive];
}


// NOTE: overlay could be NULl
- (void)displayInternal: (SDL_VoutOverlay *) overlay
{
    if (overlay != NULL) {
        _renderSize.width = overlay->w;
        _renderSize.height = overlay->h;
        [self setupEAGLContext:_context];
    }
    if (![self setupRenderer:overlay]) {
        if (!overlay && !_renderer) {
            NSLog(@"IJKSDLFboGLView: setupDisplay not ready\n");
        } else {
            NSLog(@"IJKSDLFboGLView: setupDisplay failed\n");
        }
        return;
    }
    
    IJK_GLES2_Renderer_setGravity(_renderer, 1, _renderSize.width, _renderSize.height);
    
    glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
    glClearColor(1, 0, 0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, _renderSize.width, _renderSize.height);
    
    if (!IJK_GLES2_Renderer_renderOverlay(_renderer, overlay))
        ALOGE("[EGL] IJK_GLES2_render failed\n");
    
    [_context presentRenderbuffer:GL_RENDERBUFFER];
    
    
    glFlush();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    if (_cvPBView) {
        [_cvPBView display_pixelbuffer:_target];
    }
}

- (void) lockGLActive
{
    [_glActiveLock lock];
}

- (void) unlockGLActive
{
    [_glActiveLock unlock];
}

- (BOOL) tryLockGLActive
{
    if (![_glActiveLock tryLock])
        return NO;
    
    if (_glActivePaused) {
        [_glActiveLock unlock];
        return NO;
    }
    
    return YES;
}

- (void)toggleGLPaused:(BOOL)paused
{
    [self lockGLActive];
    if (!_glActivePaused && paused) {
        if (_context != nil) {
            EAGLContext *prevContext = [EAGLContext currentContext];
            [EAGLContext setCurrentContext:_context];
            glFinish();
            [EAGLContext setCurrentContext:prevContext];
        }
    }
    _glActivePaused = paused;
    [self unlockGLActive];
}

- (void)registerApplicationObservers
{
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(applicationWillEnterForeground)
                                                 name:UIApplicationWillEnterForegroundNotification
                                               object:nil];
    [_registeredNotifications addObject:UIApplicationWillEnterForegroundNotification];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(applicationDidBecomeActive)
                                                 name:UIApplicationDidBecomeActiveNotification
                                               object:nil];
    [_registeredNotifications addObject:UIApplicationDidBecomeActiveNotification];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(applicationWillResignActive)
                                                 name:UIApplicationWillResignActiveNotification
                                               object:nil];
    [_registeredNotifications addObject:UIApplicationWillResignActiveNotification];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(applicationDidEnterBackground)
                                                 name:UIApplicationDidEnterBackgroundNotification
                                               object:nil];
    [_registeredNotifications addObject:UIApplicationDidEnterBackgroundNotification];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(applicationWillTerminate)
                                                 name:UIApplicationWillTerminateNotification
                                               object:nil];
    [_registeredNotifications addObject:UIApplicationWillTerminateNotification];
}

- (void)unregisterApplicationObservers
{
    for (NSString *name in _registeredNotifications) {
        [[NSNotificationCenter defaultCenter] removeObserver:self
                                                        name:name
                                                      object:nil];
    }
}

- (void)applicationWillEnterForeground
{
    NSLog(@"IJKSDLGLView:applicationWillEnterForeground: %d", (int)[UIApplication sharedApplication].applicationState);
    [self setupGLOnce];
    _applicationState = IJKSDLGLViewApplicationForegroundState;
    [self toggleGLPaused:NO];
}

- (void)applicationDidBecomeActive
{
    NSLog(@"IJKSDLGLView:applicationDidBecomeActive: %d", (int)[UIApplication sharedApplication].applicationState);
    [self setupGLOnce];
    [self toggleGLPaused:NO];
}

- (void)applicationWillResignActive
{
    NSLog(@"IJKSDLGLView:applicationWillResignActive: %d", (int)[UIApplication sharedApplication].applicationState);
    [self toggleGLPaused:YES];
    glFinish();
}

- (void)applicationDidEnterBackground
{
    NSLog(@"IJKSDLGLView:applicationDidEnterBackground: %d", (int)[UIApplication sharedApplication].applicationState);
    _applicationState = IJKSDLGLViewApplicationBackgroundState;
    [self toggleGLPaused:YES];
    glFinish();
}

- (void)applicationWillTerminate
{
    NSLog(@"IJKSDLGLView:applicationWillTerminate: %d", (int)[UIApplication sharedApplication].applicationState);
    [self toggleGLPaused:YES];
}


- (void)display_pixels:(IJKOverlay *)overlay
{
}

- (UIImage *)snapshot {
    return nil;
}

@end
