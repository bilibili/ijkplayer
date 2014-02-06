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
#import "../../../IJKMediaModule.h"

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
    int             _rightPaddingPixels;
    GLfloat         _rightPadding;
    int             _bytesPerPixel;

    id<IJKSDLGLRender> _renderer;

    BOOL            _didSetContentMode;
    BOOL            _didRelayoutSubViews;
    BOOL            _didPaddingChanged;
}

enum {
	ATTRIBUTE_VERTEX,
   	ATTRIBUTE_TEXCOORD,
};

+ (Class) layerClass
{
	return [CAEAGLLayer class];
}

- (id) initWithFrame:(CGRect)frame withChroma:(int)chroma
{
    self = [super initWithFrame:frame];
    if (self) {
        CAEAGLLayer *eaglLayer = (CAEAGLLayer*) self.layer;
        eaglLayer.opaque = YES;
        eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                        [NSNumber numberWithBool:NO], kEAGLDrawablePropertyRetainedBacking,
                                        kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat,
                                        nil];

        CGFloat scaleFactor = [[UIScreen mainScreen] scale];
        if (scaleFactor < 1.0f)
            scaleFactor = 1.0f;

        [eaglLayer setContentsScale:scaleFactor];

        switch (chroma) {
            case SDL_FCC_I420:
                NSLog(@"OK use I420 GL renderer");
                _frameChroma = SDL_FCC_I420;
                _renderer = [[IJKSDLGLRenderI420 alloc] init];
                _bytesPerPixel = 1;
                break;
            case SDL_FCC_RV24:
                NSLog(@"OK use RV24 GL renderer");
                _frameChroma = SDL_FCC_RV24;
                _renderer = [[IJKSDLGLRenderRV24 alloc] init];
                _bytesPerPixel = 3;
                break;
            default:
                NSLog(@"unknown chroma %d\n", chroma);
        }

        _context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];

        if (_context == nil || ![EAGLContext setCurrentContext:_context]) {
            NSLog(@"failed to setup EAGLContext");
            self = nil;
            return nil;
        }

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
            if ([EAGLContext currentContext] == _context)
                [EAGLContext setCurrentContext:nil];
            self = nil;
            return nil;
        }

        GLenum glError = glGetError();
        if (GL_NO_ERROR != glError) {
            NSLog(@"failed to setup GL %x\n", glError);
            if ([EAGLContext currentContext] == _context)
                [EAGLContext setCurrentContext:nil];
            self = nil;
            return nil;
        }

        if (![self loadShaders]) {
            if ([EAGLContext currentContext] == _context)
                [EAGLContext setCurrentContext:nil];
            self = nil;
            return nil;
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

        NSLog(@"OK setup GL");
        if ([EAGLContext currentContext] == _context)
            [EAGLContext setCurrentContext:nil];
    }

    return self;
}

- (void)dealloc
{
    _renderer = nil;

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

	if ([EAGLContext currentContext] == _context) {
		[EAGLContext setCurrentContext:nil];
	}

	_context = nil;
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
}

- (BOOL)setupDisplay: (SDL_VoutOverlay *) overlay
{
    if (_renderer && overlay && _frameChroma != overlay->format) {
        // TODO: if format changed?
    }

    if (_renderer == nil) {
        if (overlay == nil) {
            return NO;
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

    if (overlay && (_frameWidth != overlay->w || _frameHeight != overlay->h)) {
        _frameWidth = overlay->w;
        _frameHeight = overlay->h;
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
    const float width           = _frameWidth;
    const float height          = _frameHeight;
    const float dW              = (float)_backingWidth	/ width;
    const float dH              = (float)_backingHeight / height;
    float dd                    = 1.0f;
    float nW                    = 1.0f;
    float nH                    = 1.0f;

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
    // gles throws gpus_ReturnNotPermittedKillClient, while app is in background
    if (![[IJKMediaModule sharedModule] tryLockActiveApp]) {
        NSLog(@"IJKSDLGLView:display: unable to tryLock app activity\n");
        return;
    }

    [self displayInternal:overlay];

    [[IJKMediaModule sharedModule] unlockApp];
}

- (void)displayInternal: (SDL_VoutOverlay *) overlay
{
    if (![self setupDisplay:overlay]) {
        NSLog(@"IJKSDLGLView: setupDisplay failed\n");
        return;
    }

    if (_context == nil) {
        NSLog(@"IJKSDLGLView: nil EAGLContext\n");
        return;
    }

    [EAGLContext setCurrentContext:_context];

    if (overlay->pitches[0] / _bytesPerPixel > _frameWidth) {
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
        _frameWidth = overlay->w;
        _frameHeight = overlay->h;
        [_renderer display:overlay];
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
    }

    glBindRenderbuffer(GL_RENDERBUFFER, _renderbuffer);
    [_context presentRenderbuffer:GL_RENDERBUFFER];

    // Detach context before leaving display, to avoid multiple thread issues.
    if ([EAGLContext currentContext] == _context)
        [EAGLContext setCurrentContext:nil];
}

@end
