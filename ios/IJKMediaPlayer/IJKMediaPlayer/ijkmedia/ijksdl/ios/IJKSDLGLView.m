/*
 * IJKSDLGLView.m
 *
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

#import "IJKSDLGLView.h"
#import "IJKSDLGLShader.h"
#import "IJKSDLGLRender.h"
#import "IJKSDLGLRenderI420.h"
#import "IJKSDLGLRenderRV24.h"
#import "IJKSDLGLRenderNV12.h"
#include "ijksdl/ijksdl_timer.h"
#include "ijksdl/ios/ijksdl_ios.h"


static NSString *const g_vertexShaderString = IJK_SHADER_STRING
(
    attribute vec4 position;
    attribute vec2 texcoord;
    uniform mat4 modelViewProjectionMatrix;
    varying vec2 v_texcoord;

    void main()
    {
        gl_Position = modelViewProjectionMatrix * position;
        v_texcoord = texcoord.xy;
    }
);

static BOOL validateProgram(GLuint prog)
{
	GLint status;

    glValidateProgram(prog);

#ifdef DEBUG
    GLint logLength;
    glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0)
    {
        GLchar *log = (GLchar *)malloc(logLength);
        glGetProgramInfoLog(prog, logLength, &logLength, log);
        NSLog(@"Program validate log:\n%s", log);
        free(log);
    }
#endif

    glGetProgramiv(prog, GL_VALIDATE_STATUS, &status);
    if (status == GL_FALSE) {
		NSLog(@"Failed to validate program %d", prog);
        return NO;
    }

	return YES;
}

static GLuint compileShader(GLenum type, NSString *shaderString)
{
	GLint status;
	const GLchar *sources = (GLchar *)shaderString.UTF8String;

    GLuint shader = glCreateShader(type);
    if (shader == 0 || shader == GL_INVALID_ENUM) {
        NSLog(@"Failed to create shader %d", type);
        return 0;
    }

    glShaderSource(shader, 1, &sources, NULL);
    glCompileShader(shader);

#ifdef DEBUG
	GLint logLength;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0)
    {
        GLchar *log = (GLchar *)malloc(logLength);
        glGetShaderInfoLog(shader, logLength, &logLength, log);
        NSLog(@"Shader compile log:\n%s", log);
        free(log);
    }
#endif

    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
        glDeleteShader(shader);
		NSLog(@"Failed to compile shader:\n");
        return 0;
    }

	return shader;
}

static void mat4f_LoadOrtho(float left, float right, float bottom, float top, float near, float far, float* mout)
{
	float r_l = right - left;
	float t_b = top - bottom;
	float f_n = far - near;
	float tx = - (right + left) / (right - left);
	float ty = - (top + bottom) / (top - bottom);
	float tz = - (far + near) / (far - near);

	mout[0] = 2.0f / r_l;
	mout[1] = 0.0f;
	mout[2] = 0.0f;
	mout[3] = 0.0f;

	mout[4] = 0.0f;
	mout[5] = 2.0f / t_b;
	mout[6] = 0.0f;
	mout[7] = 0.0f;

	mout[8] = 0.0f;
	mout[9] = 0.0f;
	mout[10] = -2.0f / f_n;
	mout[11] = 0.0f;

	mout[12] = tx;
	mout[13] = ty;
	mout[14] = tz;
	mout[15] = 1.0f;
}


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
    GLuint          _program;
    GLint           _uniformMatrix;
    GLfloat         _vertices[8];
    GLfloat         _texCoords[8];

    int             _frameWidth;
    int             _frameHeight;
    int             _frameChroma;
    int             _frameSarNum;
    int             _frameSarDen;
    int             _rightPaddingPixels;
    GLfloat         _rightPadding;
    int             _bytesPerPixel;
    int             _frameCount;
    
    int64_t         _lastFrameTime;

    GLfloat         _prevScaleFactor;

    id<IJKSDLGLRender>        _renderer;
    CVOpenGLESTextureCacheRef _textureCache;

    BOOL            _didSetContentMode;
    BOOL            _didRelayoutSubViews;
    BOOL            _didPaddingChanged;

    int             _tryLockErrorCount;
    BOOL            _didSetupGL;
    BOOL            _didStopGL;
    NSMutableArray *_registeredNotifications;

    BOOL                _useRenderQueue;
    dispatch_queue_t    _renderQueue;
}

enum {
	ATTRIBUTE_VERTEX,
   	ATTRIBUTE_TEXCOORD,
};

static int g_ijk_gles_queue_spec_key;

+ (Class) layerClass
{
	return [CAEAGLLayer class];
}

- (id) initWithFrame:(CGRect)frame useRenderQueue:(BOOL)useRenderQueue;
{
    self = [super initWithFrame:frame];
    if (self) {
        _tryLockErrorCount = 0;

        self.glActiveLock = [[NSRecursiveLock alloc] init];
        _registeredNotifications = [[NSMutableArray alloc] init];
        [self registerApplicationObservers];

        self->_useRenderQueue = useRenderQueue;
        if (useRenderQueue) {
            dispatch_queue_attr_t attr = NULL;
            if (isIOS8OrLater()) {
                attr = dispatch_queue_attr_make_with_qos_class(DISPATCH_QUEUE_SERIAL,
                                                               QOS_CLASS_USER_INTERACTIVE,
                                                               DISPATCH_QUEUE_PRIORITY_HIGH);
            }
            _renderQueue = dispatch_queue_create("ijk-gles", attr);
            dispatch_queue_set_specific(_renderQueue,
                                        &g_ijk_gles_queue_spec_key,
                                        &g_ijk_gles_queue_spec_key,
                                        NULL);
        }

        _didSetupGL = NO;
        [self setupGLOnce];
    }

    return self;
}

- (BOOL)setupEAGLContext:(EAGLContext *)context
{
    glGenFramebuffers(1, &_framebuffer);
    glGenRenderbuffers(1, &_renderbuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, _renderbuffer);
    [_context renderbufferStorage:GL_RENDERBUFFER fromDrawable:(CAEAGLLayer*)self.layer];
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &_backingWidth);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &_backingHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _renderbuffer);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        NSLog(@"failed to make complete framebuffer object %x\n", status);
        return NO;
    }

    CVReturn err = CVOpenGLESTextureCacheCreate(kCFAllocatorDefault, NULL, _context, NULL, &_textureCache);
    if (err) {
        NSLog(@"Error at CVOpenGLESTextureCacheCreate %d\n", err);
        return NO;
    }

    GLenum glError = glGetError();
    if (GL_NO_ERROR != glError) {
        NSLog(@"failed to setup GL %x\n", glError);
        return NO;
    }

    _vertices[0] = -1.0f;  // x0
    _vertices[1] = -1.0f;  // y0
    _vertices[2] =  1.0f;  // ..
    _vertices[3] = -1.0f;
    _vertices[4] = -1.0f;
    _vertices[5] =  1.0f;
    _vertices[6] =  1.0f;  // x3
    _vertices[7] =  1.0f;  // y3

    _texCoords[0] = 0.0f;
    _texCoords[1] = 1.0f;
    _texCoords[2] = 1.0f;
    _texCoords[3] = 1.0f;
    _texCoords[4] = 0.0f;
    _texCoords[5] = 0.0f;
    _texCoords[6] = 1.0f;
    _texCoords[7] = 0.0f;

    _rightPadding = 0.0f;

    return YES;
}

- (BOOL)setupGL
{
    CAEAGLLayer *eaglLayer = (CAEAGLLayer*) self.layer;
    eaglLayer.opaque = YES;
    eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                    [NSNumber numberWithBool:NO], kEAGLDrawablePropertyRetainedBacking,
                                    kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat,
                                    nil];

    _scaleFactor = [[UIScreen mainScreen] scale];
    if (_scaleFactor < 0.1f)
        _scaleFactor = 1.0f;
    _prevScaleFactor = _scaleFactor;

    [eaglLayer setContentsScale:_scaleFactor];

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

- (BOOL)setupGLGuarded
{
    if (![self tryLockGLActive]) {
        return NO;
    }

    BOOL didSetupGL = [self setupGL];
    [self unlockGLActive];
    return didSetupGL;
}

- (BOOL)setupGLOnce
{
    if (_didSetupGL)
        return YES;

    if ([self isApplicationActive] == NO)
        return NO;

    __block BOOL didSetup = NO;
    if ([NSThread isMainThread]) {
        didSetup = [self setupGLGuarded];
    } else {
        dispatch_sync(dispatch_get_main_queue(), ^{
            didSetup = [self setupGLGuarded];
        });
    }

    return didSetup;
}

- (BOOL)isApplicationActive
{
    UIApplicationState appState = [UIApplication sharedApplication].applicationState;
    switch (appState) {
        case UIApplicationStateActive:
            return YES;
        case UIApplicationStateInactive:
        case UIApplicationStateBackground:
        default:
            return NO;
    }
}

- (void)dealloc
{
    [self lockGLActive];

    _didStopGL = YES;
    _renderer = nil;

    if (_renderQueue) {
        _renderQueue = nil;
    }

    EAGLContext *prevContext = [EAGLContext currentContext];
    [EAGLContext setCurrentContext:_context];

    if (_framebuffer) {
        glDeleteFramebuffers(1, &_framebuffer);
        _framebuffer = 0;
    }

    if (_renderbuffer) {
        glDeleteRenderbuffers(1, &_renderbuffer);
        _renderbuffer = 0;
    }

    if (_program) {
        glDeleteProgram(_program);
        _program = 0;
    }

    if (_textureCache) {
        CFRelease(_textureCache);
        _textureCache = 0;
    }

    [EAGLContext setCurrentContext:prevContext];

    _context = nil;

    [self unregisterApplicationObservers];

    [self unlockGLActive];
}

- (void)setScaleFactor:(CGFloat)scaleFactor
{
    _scaleFactor = scaleFactor;
}

- (void)layoutSubviews
{
    _didRelayoutSubViews = YES;
}

- (void)layoutOnDisplayThread
{
    glBindRenderbuffer(GL_RENDERBUFFER, _renderbuffer);
    [_context renderbufferStorage:GL_RENDERBUFFER fromDrawable:(CAEAGLLayer*)self.layer];
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &_backingWidth);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &_backingHeight);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {

        NSLog(@"failed to make complete framebuffer object %x", status);

	} else {

        NSLog(@"OK setup GL framebuffer %d:%d", _backingWidth, _backingHeight);
    }

    [self updateVertices];
    // FIXME: trigger a redisplay on display thread
    // [self display: nil];
}

- (void)setContentMode:(UIViewContentMode)contentMode
{
    _didSetContentMode = YES;
    [super setContentMode:contentMode];
    if (self->_useRenderQueue && self->_renderQueue) {
        dispatch_async(self->_renderQueue, ^(){
            [self display:nil];
        });
    }
}

- (BOOL)setupDisplay: (SDL_VoutOverlay *) overlay
{
    if (_renderer && overlay && _frameChroma != overlay->format) {
        // TODO: if format changed?
    }

    if (_renderer == nil) {
        if (overlay == nil) {
            return NO;
        } else if (overlay->format == SDL_FCC__VTB) {
            _frameChroma = overlay->format;
            _renderer = [[IJKSDLGLRenderNV12 alloc] initWithTextureCache:_textureCache];
            _bytesPerPixel = 1;
            NSLog(@"OK use NV12 GL renderer");
        } else if (overlay->format == SDL_FCC_I420) {
            _frameChroma = overlay->format;
            _renderer = [[IJKSDLGLRenderI420 alloc] init];
            _bytesPerPixel = 1;
            NSLog(@"OK use I420 GL renderer");
        } else if (overlay->format == SDL_FCC_RV24) {
            _frameChroma = overlay->format;
            _renderer = [[IJKSDLGLRenderRV24 alloc] init];
            _bytesPerPixel = 3;
            NSLog(@"OK use RV24 GL renderer");
        }

        if (![self loadShaders]) {
            return NO;
        }
    }

    if (overlay && (_frameWidth  != overlay->w ||
                    _frameHeight != overlay->h ||
                    _frameSarNum != overlay->sar_num ||
                    _frameSarDen != overlay->sar_den)) {
        _frameWidth  = overlay->w;
        _frameHeight = overlay->h;
        _frameSarNum = overlay->sar_num;
        _frameSarDen = overlay->sar_den;
        [self updateVertices];
    }

    return YES;
}

- (BOOL)loadShaders
{
    BOOL result = NO;
    GLuint vertShader = 0, fragShader = 0;

	_program = glCreateProgram();

    vertShader = compileShader(GL_VERTEX_SHADER, g_vertexShaderString);
	if (!vertShader)
        goto exit;

	fragShader = compileShader(GL_FRAGMENT_SHADER, _renderer.fragmentShader);
    if (!fragShader)
        goto exit;

	glAttachShader(_program, vertShader);
	glAttachShader(_program, fragShader);
	glBindAttribLocation(_program, ATTRIBUTE_VERTEX, "position");
    glBindAttribLocation(_program, ATTRIBUTE_TEXCOORD, "texcoord");

	glLinkProgram(_program);

    GLint status;
    glGetProgramiv(_program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) {
		NSLog(@"Failed to link program %d", _program);
        goto exit;
    }

    result = validateProgram(_program);

    _uniformMatrix = glGetUniformLocation(_program, "modelViewProjectionMatrix");
    [_renderer resolveUniforms:_program];

exit:

    if (vertShader)
        glDeleteShader(vertShader);
    if (fragShader)
        glDeleteShader(fragShader);

    if (result) {

        NSLog(@"OK setup GL programm");

    } else {

        glDeleteProgram(_program);
        _program = 0;
    }

    return result;
}

- (void)updateVertices
{
    float width                 = _frameWidth;
    float height                = _frameHeight;
    const float dW              = (float)_backingWidth	/ width;
    const float dH              = (float)_backingHeight / height;
    float dd                    = 1.0f;
    float nW                    = 1.0f;
    float nH                    = 1.0f;

    if (_frameSarNum > 0 && _frameSarDen > 0) {
        width = width * _frameSarNum / _frameSarDen;
    }

    switch (self.contentMode) {
        case UIViewContentModeScaleToFill:
            break;
        case UIViewContentModeCenter:
            nW = 1.0f / dW / [UIScreen mainScreen].scale;
            nH = 1.0f / dH / [UIScreen mainScreen].scale;
            break;
        case UIViewContentModeScaleAspectFill:
            dd = MAX(dW, dH);
            nW = (width  * dd / (float)_backingWidth );
            nH = (height * dd / (float)_backingHeight);
            break;
        case UIViewContentModeScaleAspectFit:
        default:
            dd = MIN(dW, dH);
            nW = (width  * dd / (float)_backingWidth );
            nH = (height * dd / (float)_backingHeight);
            break;
    }

    _vertices[0] = - nW;
    _vertices[1] = - nH;
    _vertices[2] =   nW;
    _vertices[3] = - nH;
    _vertices[4] = - nW;
    _vertices[5] =   nH;
    _vertices[6] =   nW;
    _vertices[7] =   nH;
}

- (void)display: (SDL_VoutOverlay *) overlay
{
    if (self->_useRenderQueue && !dispatch_get_specific(&g_ijk_gles_queue_spec_key)) {
        dispatch_sync(self->_renderQueue, ^() {
            [self display:overlay];
        });
        return;
    }

    if ([self setupGLOnce]) {
        // gles throws gpus_ReturnNotPermittedKillClient, while app is in background
        if (![self tryLockGLActive]) {
            if (0 == (_tryLockErrorCount % 100)) {
                NSLog(@"IJKSDLGLView:display: unable to tryLock GL active: %d\n", _tryLockErrorCount);
            }
            _tryLockErrorCount++;
            return;
        }

        _tryLockErrorCount = 0;
        if (!_didStopGL) {
            if (_context == nil) {
                NSLog(@"IJKSDLGLView: nil EAGLContext\n");
                return;
            }

            EAGLContext *prevContext = [EAGLContext currentContext];
            [EAGLContext setCurrentContext:_context];
            [self displayInternal:overlay];
            [EAGLContext setCurrentContext:prevContext];
        }

        [self unlockGLActive];
    }
}

- (void)displayInternal: (SDL_VoutOverlay *) overlay
{
    CGFloat newScaleFactor = _scaleFactor;
    if (_prevScaleFactor != newScaleFactor) {
        CAEAGLLayer *eaglLayer = (CAEAGLLayer*) self.layer;
        [eaglLayer setContentsScale:newScaleFactor];

        _prevScaleFactor = newScaleFactor;
    }

    if (![self setupDisplay:overlay]) {
        NSLog(@"IJKSDLGLView: setupDisplay failed\n");
        return;
    }

    if (overlay && !overlay->is_private && overlay->pitches[0] / _bytesPerPixel > _frameWidth) {
        _rightPaddingPixels = overlay->pitches[0] / _bytesPerPixel - _frameWidth;
        _didPaddingChanged = YES;
    }

    if (_didRelayoutSubViews) {
        [self layoutOnDisplayThread];
        _didRelayoutSubViews = NO;
    }

    if (_didSetContentMode || _didPaddingChanged) {
        _didSetContentMode = NO;
        _didPaddingChanged = NO;
        [self updateVertices];
    }

    glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
    glViewport(0, 0, _backingWidth, _backingHeight);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(_program);

    if (overlay) {
        _frameWidth  = overlay->w;
        _frameHeight = overlay->h;
        _frameSarNum = overlay->sar_num;
        _frameSarDen = overlay->sar_den;
        [_renderer render:overlay];
    }

    if ([_renderer prepareDisplay]) {
        if (_frameWidth > 0)
            _rightPadding = ((GLfloat)_rightPaddingPixels) / _frameWidth;

        _texCoords[0] = 0.0f;
        _texCoords[1] = 1.0f;
        _texCoords[2] = 1.0f - _rightPadding;
        _texCoords[3] = 1.0f;
        _texCoords[4] = 0.0f;
        _texCoords[5] = 0.0f;
        _texCoords[6] = 1.0f - _rightPadding;
        _texCoords[7] = 0.0f;

        GLfloat modelviewProj[16];
        mat4f_LoadOrtho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f, modelviewProj);
        glUniformMatrix4fv(_uniformMatrix, 1, GL_FALSE, modelviewProj);

        glVertexAttribPointer(ATTRIBUTE_VERTEX, 2, GL_FLOAT, 0, 0, _vertices);
        glEnableVertexAttribArray(ATTRIBUTE_VERTEX);
        glVertexAttribPointer(ATTRIBUTE_TEXCOORD, 2, GL_FLOAT, 0, 0, _texCoords);
        glEnableVertexAttribArray(ATTRIBUTE_TEXCOORD);

#if 0
        if (!validateProgram(_program))
        {
            NSLog(@"Failed to validate program");
            return;
        }
#endif

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

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
    }
}

#pragma mark AppDelegate

- (void) lockGLActive
{
    [self.glActiveLock lock];
}

- (void) unlockGLActive
{
    @synchronized(self) {
        [self.glActiveLock unlock];
    }
}

- (BOOL) tryLockGLActive
{
    if (![self.glActiveLock tryLock])
        return NO;

    /*-
    if ([UIApplication sharedApplication].applicationState != UIApplicationStateActive &&
        [UIApplication sharedApplication].applicationState != UIApplicationStateInactive) {
        [self.appLock unlock];
        return NO;
    }
     */

    if (self.glActivePaused) {
        [self.glActiveLock unlock];
        return NO;
    }
    
    return YES;
}

- (void)toggleGLPaused:(BOOL)paused
{
    [self lockGLActive];
    self.glActivePaused = paused;
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
    [self toggleGLPaused:NO];
}

- (void)applicationDidBecomeActive
{
    NSLog(@"IJKSDLGLView:applicationDidBecomeActive: %d", (int)[UIApplication sharedApplication].applicationState);
    [self toggleGLPaused:NO];
}

- (void)applicationWillResignActive
{
    NSLog(@"IJKSDLGLView:applicationWillResignActive: %d", (int)[UIApplication sharedApplication].applicationState);
    [self toggleGLPaused:YES];
}

- (void)applicationDidEnterBackground
{
    NSLog(@"IJKSDLGLView:applicationDidEnterBackground: %d", (int)[UIApplication sharedApplication].applicationState);
    [self toggleGLPaused:YES];
}

- (void)applicationWillTerminate
{
    NSLog(@"IJKSDLGLView:applicationWillTerminate: %d", (int)[UIApplication sharedApplication].applicationState);
    [self toggleGLPaused:YES];
}

#pragma mark snapshot

- (UIImage*)snapshot
{
    [self lockGLActive];

    UIImage *image = [self snapshotInternal];

    [self unlockGLActive];

    return image;
}

- (UIImage*)snapshotInternal
{
    if (isIOS7OrLater()) {
        return [self snapshotInternalOnIOS7AndLater];
    } else {
        return [self snapshotInternalOnIOS6AndBefore];
    }
}

- (UIImage*)snapshotInternalOnIOS7AndLater
{
    UIGraphicsBeginImageContextWithOptions(self.bounds.size, NO, 0.0);
    // Render our snapshot into the image context
    [self drawViewHierarchyInRect:self.bounds afterScreenUpdates:NO];

    // Grab the image from the context
    UIImage *complexViewImage = UIGraphicsGetImageFromCurrentImageContext();
    // Finish using the context
    UIGraphicsEndImageContext();

    return complexViewImage;
}

- (UIImage*)snapshotInternalOnIOS6AndBefore
{
    EAGLContext *prevContext = [EAGLContext currentContext];
    [EAGLContext setCurrentContext:_context];

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

    [EAGLContext setCurrentContext:prevContext];

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
    UIImage *image = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();

    // Clean up
    free(data);
    CFRelease(ref);
    CFRelease(colorspace);
    CGImageRelease(iref);

    return image;
}

@end
