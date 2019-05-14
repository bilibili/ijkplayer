/*
 * IJKSDLGLView.m
 *
 * Copyright (c) 2013 Bilibili
 * Copyright (c) 2013 Zhang Rui <bbcallen@gmail.com>
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

#import <AVFoundation/AVFoundation.h>

#import "IJKSDLGLView.h"
#include "ijksdl/ijksdl_timer.h"
#include "ijksdl/ios/ijksdl_ios.h"
#include "ijksdl/ijksdl_gles2.h"

typedef NS_ENUM(NSInteger, IJKSDLGLViewApplicationState) {
    IJKSDLGLViewApplicationUnknownState = 0,
    IJKSDLGLViewApplicationForegroundState = 1,
    IJKSDLGLViewApplicationBackgroundState = 2
};

@interface IJKSDLGLView()
@property(atomic,strong) NSRecursiveLock *glActiveLock;
@property(atomic) BOOL glActivePaused;
@end

@implementation IJKSDLGLView {
    EAGLContext     *_context;
    GLuint          _framebuffer;
    GLuint          _renderbuffer;
    GLint           _backingWidth;
    GLint           _backingHeight;
    
    int             _frameCount;
    
    int64_t         _lastFrameTime;
    
    IJK_GLES2_Renderer *_renderer;
    int                 _rendererGravity;
    
    BOOL            _isRenderBufferInvalidated;
    
    BOOL            _isGLInitialized;
    NSMutableArray *_registeredNotifications;
    
    IJKSDLGLViewApplicationState _applicationState;
    CGSize _frameSize;
}

@synthesize isThirdGLView              = _isThirdGLView;
@synthesize scaleFactor                = _scaleFactor;
@synthesize fps                        = _fps;
@synthesize videoPaused                = _videoPaused;

+ (Class) layerClass {
    return [CAEAGLLayer class];
}

#pragma mark - initializers/deinitializer

- (id) initWithFrame:(CGRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
        _frameSize = CGSizeZero;
        self.glActiveLock = [[NSRecursiveLock alloc] init];
        _registeredNotifications = [[NSMutableArray alloc] init];
        [self registerApplicationObservers];
        [self initializeProximityMonitor];
        
        [self updateApplicationState];
        
        _isGLInitialized = NO;
        [self setupGLOnce];
    }
    
    return self;
}

- (void)dealloc {
    [self synchronize:^{
        _isGLInitialized = NO;
        [self exec_in_eaglcontext:^{
            IJK_GLES2_Renderer_reset(_renderer);
            IJK_GLES2_Renderer_freeP(&_renderer);
            
            if (_framebuffer) {
                glDeleteFramebuffers(1, &_framebuffer);
                _framebuffer = 0;
            }
            
            if (_renderbuffer) {
                glDeleteRenderbuffers(1, &_renderbuffer);
                _renderbuffer = 0;
            }
            
            glFinish();
        }];
        _context = nil;
        
        [self unregisterApplicationObservers];
        [self deInitializeProximityMonitor];
    }];
}

#pragma mark - public methods

- (void)layoutSubviews {
    [super layoutSubviews];
    
    CGFloat newScaleFactor = _scaleFactor;
    if (self.window.screen != nil) {
        newScaleFactor = self.window.screen.scale;
    }
    
    if (newScaleFactor != _scaleFactor || !CGSizeEqualToSize(self.frame.size, _frameSize)) {
        _frameSize = self.frame.size;
        _scaleFactor = newScaleFactor;
        
        [self setIsRenderBufferInvalidated:YES];
    }
}

- (void)setScaleFactor:(CGFloat)scaleFactor {
    if (_scaleFactor != scaleFactor) {
        _scaleFactor = scaleFactor;
        
        [self setIsRenderBufferInvalidated:YES];
    }
}

- (void)setContentMode:(UIViewContentMode)contentMode {
    [super setContentMode:contentMode];
    
    int newRendererGravity = _rendererGravity;
    switch (contentMode) {
        case UIViewContentModeScaleToFill:
            newRendererGravity = IJK_GLES2_GRAVITY_RESIZE;
            break;
        case UIViewContentModeScaleAspectFit:
            newRendererGravity = IJK_GLES2_GRAVITY_RESIZE_ASPECT;
            break;
        case UIViewContentModeScaleAspectFill:
            newRendererGravity = IJK_GLES2_GRAVITY_RESIZE_ASPECT_FILL;
            break;
        default:
            newRendererGravity = IJK_GLES2_GRAVITY_RESIZE_ASPECT;
            break;
    }
    if (newRendererGravity != _rendererGravity) {
        [self setIsRenderBufferInvalidated:YES];
    }
}

- (void) display_pixels:(IJKOverlay *)overlay {
    return;
}

- (void)display:(SDL_VoutOverlay *)overlay {
    [self trySynchronize:^{
        [self displayInternal:overlay];
    }];
}

- (void)setVideoPaused:(BOOL)videoPaused {
    [self exec_async_in_global_queue:^{
        [self synchronize:^{
            _videoPaused = videoPaused;
            [self toggleGLPaused:_videoPaused];
            if (!_videoPaused) {
                [self setupGLOnce];
            }
        }];
    }];
}

- (BOOL)videoPaused {
    __block BOOL isPaused = FALSE;
    [self synchronize:^{
        isPaused = _videoPaused;
    }];
    return isPaused;
}

- (UIImage*)snapshot {
    __block UIImage *image = NULL;
    [self synchronize:^{
        image = [self snapshotInternal];
    }];
    return image;
}

- (void)setShouldLockWhileBeingMovedToWindow:(BOOL)shouldLockWhileBeingMovedToWindow {
}

#pragma mark - helpers

- (CAEAGLLayer *)eaglLayer {
    return (CAEAGLLayer*) self.layer;
}

- (BOOL)setupEAGLContext:(EAGLContext *)context {
    glGenFramebuffers(1, &_framebuffer);
    glGenRenderbuffers(1, &_renderbuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, _renderbuffer);
    [self exec_sync_in_main_queue:^{
        [_context renderbufferStorage:GL_RENDERBUFFER fromDrawable:(CAEAGLLayer*)self.layer];
    }];
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &_backingWidth);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &_backingHeight);
    printf("GL RECT %d x %d\n", _backingWidth, _backingHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _renderbuffer);
    
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        NSLog(@"failed to make complete framebuffer object %x\n", status);
        return NO;
    }
    
    GLenum glError = glGetError();
    if (GL_NO_ERROR != glError) {
        NSLog(@"failed to setup GL %x\n", glError);
        return NO;
    }
    
    return YES;
}

- (BOOL)setupGL {
    if (_isGLInitialized) {
        return YES;
    }
    
    if (![self isApplicationActive]) {
        return NO;
    }
    
    if (self.glActivePaused) {
        return NO;
    }
    
    [self exec_sync_in_main_queue:^{
        CAEAGLLayer *eaglLayer = (CAEAGLLayer*) self.layer;
        eaglLayer.opaque = YES;
        eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                        [NSNumber numberWithBool:NO], kEAGLDrawablePropertyRetainedBacking,
                                        kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat,
                                        nil];
        
        _scaleFactor = [[UIScreen mainScreen] scale];
        if (_scaleFactor < 0.1f) {
            _scaleFactor = 1.0f;
        }
        
        [eaglLayer setContentsScale:_scaleFactor];
    }];
    
    _context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    if (_context == nil) {
        NSLog(@"failed to setup EAGLContext\n");
        return NO;
    }
    
    _isGLInitialized = NO;
    [self exec_in_eaglcontext:^{
        if ([self setupEAGLContext:_context]) {
            NSLog(@"OK setup GL\n");
            _isGLInitialized = YES;
        }
    }];
    return _isGLInitialized;
}

- (BOOL)setupGLOnce {
    __block BOOL result = NO;
    [self synchronize:^{
        if (_isGLInitialized) {
            result = YES;
        }
        
        if (![self isApplicationActive]) {
            result = NO;
        } else {
            result = [self setupGL];
        }
    }];
    return result;
}

- (void)toggleGLPaused:(BOOL)paused {
    [self synchronize:^{
        if (!self.glActivePaused && paused) {
            [self exec_in_eaglcontext:^{
                glFinish();
            }];
        }
        self.glActivePaused = paused;
    }];
}

- (BOOL)setupRenderer: (SDL_VoutOverlay *) overlay {
    if (overlay == nil) {
        return _renderer != nil;
    }
    
    if (!IJK_GLES2_Renderer_isValid(_renderer) ||
        !IJK_GLES2_Renderer_isFormat(_renderer, overlay->format)) {
        
        IJK_GLES2_Renderer_reset(_renderer);
        IJK_GLES2_Renderer_freeP(&_renderer);
        
        _renderer = IJK_GLES2_Renderer_create(overlay);
        if (!IJK_GLES2_Renderer_isValid(_renderer))
            return NO;
        
        if (!IJK_GLES2_Renderer_use(_renderer))
            return NO;
        
        IJK_GLES2_Renderer_setGravity(_renderer, _rendererGravity, _backingWidth, _backingHeight);
    }
    
    return YES;
}

- (void)setIsRenderBufferInvalidated:(BOOL)isInvalidated {
    [self exec_async_in_global_queue:^{
        [self trySynchronize:^{
            _isRenderBufferInvalidated = isInvalidated;
            if (_isRenderBufferInvalidated) {
                [self displayInternal:nil];
            }
        }];
    }];
}

- (void)displayInternal: (SDL_VoutOverlay *) overlay {
    if (!_isGLInitialized ||
        ![self isApplicationActive] || self.glActivePaused ) {
        return;
    }
    
    [self exec_in_eaglcontext:^{
        if (![self setupRenderer:overlay]) {
            if (!overlay && !_renderer) {
                NSLog(@"IJKSDLGLView: setupDisplay not ready\n");
            } else {
                NSLog(@"IJKSDLGLView: setupDisplay failed\n");
            }
            return;
        }
        
        if (self->_isRenderBufferInvalidated) {
            NSLog(@"IJKSDLGLView: renderbufferStorage fromDrawable\n");
            self->_isRenderBufferInvalidated = NO;
            
            glBindRenderbuffer(GL_RENDERBUFFER, _renderbuffer);
            [self exec_sync_in_main_queue:^{
                [[self eaglLayer] setContentsScale:_scaleFactor];
                [_context renderbufferStorage:GL_RENDERBUFFER fromDrawable:(CAEAGLLayer*)self.layer];
            }];
            
            glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &_backingWidth);
            glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &_backingHeight);
            IJK_GLES2_Renderer_setGravity(_renderer, _rendererGravity, _backingWidth, _backingHeight);
            printf("GL RECT %d x %d\n", _backingWidth, _backingHeight);
        }
        
        glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
        glViewport(0, 0, _backingWidth, _backingHeight);
        
        if (!IJK_GLES2_Renderer_renderOverlay(_renderer, overlay))
            ALOGE("[EGL] IJK_GLES2_render failed\n");
        
        glBindRenderbuffer(GL_RENDERBUFFER, _renderbuffer);
        [_context presentRenderbuffer:GL_RENDERBUFFER];
        
        int64_t current = (int64_t)SDL_GetTickHR();
        int64_t delta   = (current > _lastFrameTime) ? current - _lastFrameTime : 0;
        if (delta <= 0) {
            _lastFrameTime = current;
        } else if (delta >= 1000) {
            _fps = ((CGFloat)_frameCount) * 1000 / delta;
            _frameCount = 0;
            _lastFrameTime = current;
        } else {
            _frameCount++;
        }
    }];
}

- (void)registerApplicationObservers {
    
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

- (void)initializeProximityMonitor {
    [UIDevice.currentDevice setProximityMonitoringEnabled:![self isSpeakerEnabled]];
    if (UIDevice.currentDevice.isProximityMonitoringEnabled) {
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(proximityStateDidChange)
                                                     name:UIDeviceProximityStateDidChangeNotification
                                                   object:nil];
        [_registeredNotifications addObject:UIDeviceProximityStateDidChangeNotification];
    }
}

- (void)deInitializeProximityMonitor {
    [UIDevice.currentDevice setProximityMonitoringEnabled:NO];
}

- (UIImage*)snapshotInternal {
    if (isIOS7OrLater()) {
        return [self snapshotInternalOnIOS7AndLater];
    } else {
        return [self snapshotInternalOnIOS6AndBefore];
    }
}

- (UIImage*)snapshotInternalOnIOS7AndLater {
    if (CGSizeEqualToSize(self.bounds.size, CGSizeZero)) {
        return nil;
    }
    UIGraphicsBeginImageContextWithOptions(self.bounds.size, NO, 0.0);
    // Render our snapshot into the image context
    [self drawViewHierarchyInRect:self.bounds afterScreenUpdates:NO];
    
    // Grab the image from the context
    UIImage *complexViewImage = UIGraphicsGetImageFromCurrentImageContext();
    // Finish using the context
    UIGraphicsEndImageContext();
    
    return complexViewImage;
}

- (UIImage *)snapshotInternalOnIOS6AndBefore {
    __block UIImage *image;
    [self exec_in_eaglcontext:^{
        GLint backingWidth, backingHeight;
        
        // Bind the color renderbuffer used to render the OpenGL ES view
        // If your application only creates a single color renderbuffer which is already bound at this point,
        // this call is redundant, but it is needed if you're dealing with multiple renderbuffers.
        // Note, replace "viewRenderbuffer" with the actual name of the renderbuffer object defined in your class.
        glBindRenderbuffer(GL_RENDERBUFFER, _renderbuffer);
        
        // Get the size of the backing CAEAGLLayer
        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &backingWidth);
        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &backingHeight);
        
        NSInteger x = 0, y = 0, width = backingWidth, height = backingHeight;
        NSInteger dataLength = width * height * 4;
        GLubyte *data = (GLubyte*)malloc(dataLength * sizeof(GLubyte));
        
        // Read pixel data from the framebuffer
        glPixelStorei(GL_PACK_ALIGNMENT, 4);
        glReadPixels((int)x, (int)y, (int)width, (int)height, GL_RGBA, GL_UNSIGNED_BYTE, data);
        
        // Create a CGImage with the pixel data
        // If your OpenGL ES content is opaque, use kCGImageAlphaNoneSkipLast to ignore the alpha channel
        // otherwise, use kCGImageAlphaPremultipliedLast
        CGDataProviderRef ref = CGDataProviderCreateWithData(NULL, data, dataLength, NULL);
        CGColorSpaceRef colorspace = CGColorSpaceCreateDeviceRGB();
        CGImageRef iref = CGImageCreate(width, height, 8, 32, width * 4, colorspace, kCGBitmapByteOrder32Big | kCGImageAlphaPremultipliedLast,
                                        ref, NULL, true, kCGRenderingIntentDefault);
        
        // OpenGL ES measures data in PIXELS
        // Create a graphics context with the target size measured in POINTS
        UIGraphicsBeginImageContext(CGSizeMake(width, height));
        
        CGContextRef cgcontext = UIGraphicsGetCurrentContext();
        // UIKit coordinate system is upside down to GL/Quartz coordinate system
        // Flip the CGImage by rendering it to the flipped bitmap context
        // The size of the destination area is measured in POINTS
        CGContextSetBlendMode(cgcontext, kCGBlendModeCopy);
        CGContextDrawImage(cgcontext, CGRectMake(0.0, 0.0, width, height), iref);
        
        // Retrieve the UIImage from the current context
        image = UIGraphicsGetImageFromCurrentImageContext();
        UIGraphicsEndImageContext();
        
        // Clean up
        free(data);
        CFRelease(ref);
        CFRelease(colorspace);
        CGImageRelease(iref);
    }];
    return image;
}

- (BOOL)isSpeakerEnabled {
    __block BOOL isenabled = FALSE;
    [AVAudioSession.sharedInstance.currentRoute.outputs enumerateObjectsUsingBlock:^(AVAudioSessionPortDescription * _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
        if ([obj.portType isEqualToString:AVAudioSessionPortBuiltInSpeaker]) {
            isenabled = TRUE;
            *stop = TRUE;
        }
    }];
    return isenabled;
}

- (BOOL)isApplicationActive {
    __block BOOL result = NO;
    [self exec_sync_in_main_queue:^{
        switch (_applicationState) {
            case IJKSDLGLViewApplicationForegroundState:
                result = YES;
                break;
            default:
                result = NO;
                break;
        }
    }];
    return result;
}

- (void)updateApplicationState {
    [self exec_sync_in_main_queue:^{
        UIApplicationState appState = [UIApplication sharedApplication].applicationState;
        switch (appState) {
            case UIApplicationStateActive:
                _applicationState = IJKSDLGLViewApplicationForegroundState;
                break;
            case UIApplicationStateInactive:
            case UIApplicationStateBackground:
            default:
                _applicationState = IJKSDLGLViewApplicationBackgroundState;
                break;
        }
    }];
}

#pragma mark - queue|sync methods

- (void)synchronize:(dispatch_block_t) block {
    [self.glActiveLock lock];
    block();
    [self.glActiveLock unlock];
}

- (void)trySynchronize:(dispatch_block_t) block {
    if ([self.glActiveLock tryLock]) {
        block();
        [self.glActiveLock unlock];
    }
}

- (void)exec_in_eaglcontext:(dispatch_block_t)block {
    if (_context != nil) {
        EAGLContext *prevContext = [EAGLContext currentContext];
        [EAGLContext setCurrentContext:_context];
        block();
        [EAGLContext setCurrentContext:prevContext];
    }
}

- (void)exec_sync_in_main_queue:(dispatch_block_t)block {
    if ([NSThread isMainThread]) {
        block();
    } else {
        dispatch_sync(dispatch_get_main_queue(), ^{
            block();
        });
    }
}

- (void)exec_async_in_global_queue:(dispatch_block_t)block {
    if (![NSThread isMainThread]) {
        block();
    } else {
        dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0), ^{
            block();
        });
    }
}

#pragma mark - notifications handlers

- (void)applicationWillEnterForeground {
    NSLog(@"IJKSDLGLView:applicationWillEnterForeground: %d", (int)[UIApplication sharedApplication].applicationState);
    [self exec_async_in_global_queue:^{
        [self updateApplicationState];
        if (!self.videoPaused) {
            [self toggleGLPaused:NO];
            [self setupGLOnce];
        }
    }];
}

- (void)applicationDidBecomeActive {
    NSLog(@"IJKSDLGLView:applicationDidBecomeActive: %d", (int)[UIApplication sharedApplication].applicationState);
    [self exec_async_in_global_queue:^{
        [self updateApplicationState];
        if (!self.videoPaused) {
            [self toggleGLPaused:NO];
            [self setupGLOnce];
        }
    }];
}

- (void)applicationWillResignActive {
    NSLog(@"IJKSDLGLView:applicationWillResignActive: %d", (int)[UIApplication sharedApplication].applicationState);
    [self exec_async_in_global_queue:^{
        [self updateApplicationState];
        [self toggleGLPaused:YES];
    }];
}

- (void)applicationDidEnterBackground {
    NSLog(@"IJKSDLGLView:applicationDidEnterBackground: %d", (int)[UIApplication sharedApplication].applicationState);
    [self exec_async_in_global_queue:^{
        [self updateApplicationState];
        [self toggleGLPaused:YES];
    }];
}

- (void)applicationWillTerminate {
    NSLog(@"IJKSDLGLView:applicationWillTerminate: %d", (int)[UIApplication sharedApplication].applicationState);
    [self exec_async_in_global_queue:^{
        [self updateApplicationState];
        [self toggleGLPaused:YES];
    }];
}

- (void)proximityStateDidChange {
    NSLog(@"IJKSDLGLView:proximityStateDidChange: %d", (int)[UIApplication sharedApplication].applicationState);
    [self exec_async_in_global_queue:^{
        [self updateApplicationState];
        if ([self isApplicationActive]) {
            [self toggleGLPaused:YES];
        } else if (!self.videoPaused) {
            [self toggleGLPaused:NO];
            [self setupGLOnce];
        }
    }];
}

@end
