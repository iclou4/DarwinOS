/*
 * OpenGL.c - ravynOS OpenGL & CGL compatibility layer
 */

#include <stdlib.h>
#include <string.h>
#include <OpenGL/OpenGL.h>

struct _CGLContextObj {
    int retainCount;
    CGLPixelFormatObj pixelFormat;
    GLint backingOrigin[2];
    GLint backingSize[2];
    GLint opacity;
};

struct _CGLPixelFormatObj {
    int retainCount;
    uint32_t attributes[64];
};

struct _CGLPBufferObj {
    int retainCount;
    GLsizei width;
    GLsizei height;
    GLenum target;
};

static _Thread_local CGLContextObj s_currentContext = NULL;

CGLError CGLCreateContext(CGLPixelFormatObj pixelFormat, CGLContextObj share, CGLContextObj *result) {
    (void)share;
    if (!result) return kCGLBadAlloc;
    CGLContextObj ctx = (CGLContextObj)calloc(1, sizeof(struct _CGLContextObj));
    if (!ctx) return kCGLBadAlloc;
    ctx->retainCount = 1;
    ctx->pixelFormat = pixelFormat ? CGLRetainPixelFormat(pixelFormat) : NULL;
    *result = ctx;
    return kCGLNoError;
}

CGLContextObj CGLRetainContext(CGLContextObj context) {
    if (context) context->retainCount++;
    return context;
}

void CGLReleaseContext(CGLContextObj context) {
    if (!context) return;
    if (--context->retainCount <= 0) {
        if (context->pixelFormat) CGLReleasePixelFormat(context->pixelFormat);
        if (s_currentContext == context) s_currentContext = NULL;
        free(context);
    }
}

GLuint CGLGetContextRetainCount(CGLContextObj context) {
    return context ? (GLuint)context->retainCount : 0;
}

CGLError CGLDestroyContext(CGLContextObj context) {
    CGLReleaseContext(context);
    return kCGLNoError;
}

CGLError CGLLockContext(CGLContextObj context) {
    (void)context;
    return kCGLNoError;
}

CGLError CGLUnlockContext(CGLContextObj context) {
    (void)context;
    return kCGLNoError;
}

CGLError CGLSetParameter(CGLContextObj context, CGLContextParameter parameter, const GLint *value) {
    if (!context || !value) return kCGLBadValue;
    switch (parameter) {
        case kCGLCPSurfaceBackingOrigin:
            context->backingOrigin[0] = value[0];
            context->backingOrigin[1] = value[1];
            break;
        case kCGLCPSurfaceBackingSize:
            context->backingSize[0] = value[0];
            context->backingSize[1] = value[1];
            break;
        case kCGLCPSurfaceOpacity:
            context->opacity = *value;
            break;
        default:
            break;
    }
    return kCGLNoError;
}

CGLError CGLGetParameter(CGLContextObj context, CGLContextParameter parameter, GLint *value) {
    if (!context || !value) return kCGLBadValue;
    switch (parameter) {
        case kCGLCPSurfaceBackingOrigin:
            value[0] = context->backingOrigin[0];
            value[1] = context->backingOrigin[1];
            break;
        case kCGLCPSurfaceBackingSize:
            value[0] = context->backingSize[0];
            value[1] = context->backingSize[1];
            break;
        case kCGLCPSurfaceOpacity:
            *value = context->opacity;
            break;
        default:
            *value = 0;
            break;
    }
    return kCGLNoError;
}

CGLError CGLFlushDrawable(CGLContextObj context) {
    (void)context;
    return kCGLNoError;
}

CGLError CGLChoosePixelFormat(const CGLPixelFormatAttribute *attributes, CGLPixelFormatObj *pixelFormatp, GLint *numberOfScreensp) {
    if (!pixelFormatp) return kCGLBadValue;
    CGLPixelFormatObj pf = (CGLPixelFormatObj)calloc(1, sizeof(struct _CGLPixelFormatObj));
    if (!pf) return kCGLBadAlloc;
    pf->retainCount = 1;
    if (attributes) {
        int i = 0;
        while (attributes[i] != 0 && i < 63) {
            pf->attributes[i] = attributes[i];
            i++;
        }
    }
    *pixelFormatp = pf;
    if (numberOfScreensp) *numberOfScreensp = 1;
    return kCGLNoError;
}

CGLPixelFormatObj CGLRetainPixelFormat(CGLPixelFormatObj pixelFormat) {
    if (pixelFormat) pixelFormat->retainCount++;
    return pixelFormat;
}

void CGLReleasePixelFormat(CGLPixelFormatObj pixelFormat) {
    if (!pixelFormat) return;
    if (--pixelFormat->retainCount <= 0) {
        free(pixelFormat);
    }
}

CGLError CGLDestroyPixelFormat(CGLPixelFormatObj pixelFormat) {
    CGLReleasePixelFormat(pixelFormat);
    return kCGLNoError;
}

GLuint CGLGetPixelFormatRetainCount(CGLPixelFormatObj pixelFormat) {
    return pixelFormat ? (GLuint)pixelFormat->retainCount : 0;
}

CGLError CGLDescribePixelFormat(CGLPixelFormatObj pixelFormat, GLint screenNumber, CGLPixelFormatAttribute attribute, GLint *valuep) {
    (void)pixelFormat; (void)screenNumber; (void)attribute;
    if (valuep) *valuep = 0;
    return kCGLNoError;
}

CGLError CGLCreatePBuffer(GLsizei width, GLsizei height, GLenum target, GLenum internalFormat, GLint maxDetail, CGLPBufferObj *pbuffer) {
    (void)internalFormat; (void)maxDetail;
    if (!pbuffer) return kCGLBadAlloc;
    CGLPBufferObj pb = (CGLPBufferObj)calloc(1, sizeof(struct _CGLPBufferObj));
    if (!pb) return kCGLBadAlloc;
    pb->retainCount = 1;
    pb->width = width;
    pb->height = height;
    pb->target = target;
    *pbuffer = pb;
    return kCGLNoError;
}

CGLError CGLDescribePBuffer(CGLPBufferObj pbuffer, GLsizei *width, GLsizei *height, GLenum *target, GLenum *internalFormat, GLint *mipmap) {
    if (!pbuffer) return kCGLBadValue;
    if (width) *width = pbuffer->width;
    if (height) *height = pbuffer->height;
    if (target) *target = pbuffer->target;
    if (internalFormat) *internalFormat = 0x1908; // GL_RGBA
    if (mipmap) *mipmap = 0;
    return kCGLNoError;
}

CGLPBufferObj CGLRetainPBuffer(CGLPBufferObj pbuffer) {
    if (pbuffer) pbuffer->retainCount++;
    return pbuffer;
}

void CGLReleasePBuffer(CGLPBufferObj pbuffer) {
    if (!pbuffer) return;
    if (--pbuffer->retainCount <= 0) free(pbuffer);
}

GLuint CGLGetPBufferRetainCount(CGLPBufferObj pbuffer) {
    return pbuffer ? (GLuint)pbuffer->retainCount : 0;
}

CGLError CGLDestroyPBuffer(CGLPBufferObj pbuffer) {
    CGLReleasePBuffer(pbuffer);
    return kCGLNoError;
}

CGLError CGLGetPBuffer(CGLContextObj context, CGLPBufferObj *pbuffer, GLenum *face, GLint *level, GLint *screen) {
    (void)context; (void)pbuffer; (void)face; (void)level; (void)screen;
    return kCGLNoError;
}

CGLError CGLSetPBuffer(CGLContextObj context, CGLPBufferObj pbuffer, GLenum face, GLint level, GLint screen) {
    (void)context; (void)pbuffer; (void)face; (void)level; (void)screen;
    return kCGLNoError;
}

CGLError CGLTexImagePBuffer(CGLContextObj context, CGLPBufferObj pbuffer, GLenum sourceBuffer) {
    (void)context; (void)pbuffer; (void)sourceBuffer;
    return kCGLNoError;
}

CGLContextObj CGLGetCurrentContext(void) {
    return s_currentContext;
}

CGLError CGLSetCurrentContext(CGLContextObj context) {
    s_currentContext = context;
    return kCGLNoError;
}

void CGLUseShaders(CGLContextObj context) {
    (void)context;
}

/* OpenGL functions */
void glAlphaFunc(GLenum func, GLclampf ref) { (void)func; (void)ref; }
void glBindBuffer(GLenum target, GLuint buffer) { (void)target; (void)buffer; }
void glBindTexture(GLenum target, GLuint texture) { (void)target; (void)texture; }
void glBlendFunc(GLenum sfactor, GLenum dfactor) { (void)sfactor; (void)dfactor; }
void glBufferData(GLenum target, GLsizeiptr size, const void *data, GLenum usage) {
    (void)target; (void)size; (void)data; (void)usage;
}
void glClear(GLbitfield mask) { (void)mask; }
void glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) {
    (void)red; (void)green; (void)blue; (void)alpha;
}
void glColor4f(float red, float green, float blue, float alpha) {
    (void)red; (void)green; (void)blue; (void)alpha;
}
void glDeleteBuffers(GLsizei n, const GLuint *buffers) { (void)n; (void)buffers; }
void glDeleteTextures(GLsizei n, const GLuint *textures) { (void)n; (void)textures; }
void glDeleteVertexArrays(GLsizei n, const GLuint *arrays) { (void)n; (void)arrays; }
void glDepthFunc(GLenum func) { (void)func; }
void glDrawArrays(GLenum mode, GLint first, GLsizei count) { (void)mode; (void)first; (void)count; }
void glEnable(GLenum cap) { (void)cap; }
void glEnableVertexAttribArray(GLuint index) { (void)index; }
void glFlush(void) {}
void glGenBuffers(GLsizei n, GLuint *buffers) {
    if (buffers) for (GLsizei i = 0; i < n; i++) buffers[i] = i + 1;
}
void glGenTextures(GLsizei n, GLuint *textures) {
    if (textures) for (GLsizei i = 0; i < n; i++) textures[i] = i + 1;
}
void glGenVertexArrays(GLsizei n, GLuint *arrays) {
    if (arrays) for (GLsizei i = 0; i < n; i++) arrays[i] = i + 1;
}
GLboolean glIsTexture(GLuint texture) { (void)texture; return GL_TRUE; }
void glLoadIdentity(void) {}
void glMatrixMode(GLenum mode) { (void)mode; }
void glOrtho(double left, double right, double bottom, double top, double nearVal, double farVal) {
    (void)left; (void)right; (void)bottom; (void)top; (void)nearVal; (void)farVal;
}
void glPopMatrix(void) {}
void glPushMatrix(void) {}
void glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const void *pointer) {
    (void)size; (void)type; (void)stride; (void)pointer;
}
void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels) {
    (void)target; (void)level; (void)internalformat; (void)width; (void)height; (void)border; (void)format; (void)type; (void)pixels;
}
void glTexParameteri(GLenum target, GLenum pname, GLint param) { (void)target; (void)pname; (void)param; }
void glTranslatef(float x, float y, float z) { (void)x; (void)y; (void)z; }
void glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer) {
    (void)index; (void)size; (void)type; (void)normalized; (void)stride; (void)pointer;
}
void glVertexPointer(GLint size, GLenum type, GLsizei stride, const void *pointer) {
    (void)size; (void)type; (void)stride; (void)pointer;
}
void glViewport(GLint x, GLint y, GLsizei width, GLsizei height) {
    (void)x; (void)y; (void)width; (void)height;
}
