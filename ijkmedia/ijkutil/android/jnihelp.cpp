/*
 * Copyright (C) 2006 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "jnihelp.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "loghelp.h"

/**
 * Equivalent to ScopedLocalRef, but for C_JNIEnv instead. (And slightly more powerful.)
 */
template<typename T>
class scoped_local_ref {
public:
    scoped_local_ref(C_JNIEnv* env, T localRef = NULL)
    : mEnv(env), mLocalRef(localRef)
    {
    }

    ~scoped_local_ref() {
        reset();
    }

    void reset(T localRef = NULL) {
        if (mLocalRef != NULL) {
            (*mEnv)->DeleteLocalRef(reinterpret_cast<JNIEnv*>(mEnv), mLocalRef);
            mLocalRef = localRef;
        }
    }

    T get() const {
        return mLocalRef;
    }

private:
    C_JNIEnv* mEnv;
    T mLocalRef;

    // Disallow copy and assignment.
    scoped_local_ref(const scoped_local_ref&);
    void operator=(const scoped_local_ref&);
};

static jclass findClass(C_JNIEnv* env, const char* className) {
    JNIEnv* e = reinterpret_cast<JNIEnv*>(env);
    return (*env)->FindClass(e, className);
}

extern "C" int jniRegisterNativeMethods(C_JNIEnv* env, const char* className,
    const JNINativeMethod* gMethods, int numMethods)
{
    JNIEnv* e = reinterpret_cast<JNIEnv*>(env);

    ALOGV("Registering %s natives", className);

    scoped_local_ref<jclass> c(env, findClass(env, className));
    if (c.get() == NULL) {
        char* msg;
        asprintf(&msg, "Native registration unable to find class '%s', aborting", className);
        e->FatalError(msg);
    }

    if ((*env)->RegisterNatives(e, c.get(), gMethods, numMethods) < 0) {
        char* msg;
        asprintf(&msg, "RegisterNatives failed for '%s', aborting", className);
        e->FatalError(msg);
    }

    return 0;
}

/*
 * Returns a human-readable summary of an exception object.  The buffer will
 * be populated with the "binary" class name and, if present, the
 * exception message.
 */
static char* getExceptionSummary0(C_JNIEnv* env, jthrowable exception) {
    JNIEnv* e = reinterpret_cast<JNIEnv*>(env);

    /* get the name of the exception's class */
    scoped_local_ref<jclass> exceptionClass(env, (*env)->GetObjectClass(e, exception)); // can't fail
    scoped_local_ref<jclass> classClass(env,
            (*env)->GetObjectClass(e, exceptionClass.get())); // java.lang.Class, can't fail
    jmethodID classGetNameMethod =
            (*env)->GetMethodID(e, classClass.get(), "getName", "()Ljava/lang/String;");
    scoped_local_ref<jstring> classNameStr(env,
            (jstring) (*env)->CallObjectMethod(e, exceptionClass.get(), classGetNameMethod));
    if (classNameStr.get() == NULL) {
        return NULL;
    }

    /* get printable string */
    const char* classNameChars = (*env)->GetStringUTFChars(e, classNameStr.get(), NULL);
    if (classNameChars == NULL) {
        return NULL;
    }

    /* if the exception has a detail message, get that */
    jmethodID getMessage =
            (*env)->GetMethodID(e, exceptionClass.get(), "getMessage", "()Ljava/lang/String;");
    scoped_local_ref<jstring> messageStr(env,
            (jstring) (*env)->CallObjectMethod(e, exception, getMessage));
    if (messageStr.get() == NULL) {
        return strdup(classNameChars);
    }

    char* result = NULL;
    const char* messageChars = (*env)->GetStringUTFChars(e, messageStr.get(), NULL);
    if (messageChars != NULL) {
        asprintf(&result, "%s: %s", classNameChars, messageChars);
        (*env)->ReleaseStringUTFChars(e, messageStr.get(), messageChars);
    } else {
        (*env)->ExceptionClear(e); // clear OOM
        asprintf(&result, "%s: <error getting message>", classNameChars);
    }

    (*env)->ReleaseStringUTFChars(e, classNameStr.get(), classNameChars);
    return result;
}

static char* getExceptionSummary(C_JNIEnv* env, jthrowable exception) {
    JNIEnv* e = reinterpret_cast<JNIEnv*>(env);
    char* result = getExceptionSummary0(env, exception);
    if (result == NULL) {
        (*env)->ExceptionClear(e);
        result = strdup("<error getting class name>");
    }
    return result;
}

/*
 * Returns an exception (with stack trace) as a string.
 */
static char* getStackTrace(C_JNIEnv* env, jthrowable exception) {
    JNIEnv* e = reinterpret_cast<JNIEnv*>(env);

    scoped_local_ref<jclass> stringWriterClass(env, findClass(env, "java/io/StringWriter"));
    if (stringWriterClass.get() == NULL) {
        return NULL;
    }

    jmethodID stringWriterCtor = (*env)->GetMethodID(e, stringWriterClass.get(), "<init>", "()V");
    jmethodID stringWriterToStringMethod =
            (*env)->GetMethodID(e, stringWriterClass.get(), "toString", "()Ljava/lang/String;");

    scoped_local_ref<jclass> printWriterClass(env, findClass(env, "java/io/PrintWriter"));
    if (printWriterClass.get() == NULL) {
        return NULL;
    }

    jmethodID printWriterCtor =
            (*env)->GetMethodID(e, printWriterClass.get(), "<init>", "(Ljava/io/Writer;)V");

    scoped_local_ref<jobject> stringWriter(env,
            (*env)->NewObject(e, stringWriterClass.get(), stringWriterCtor));
    if (stringWriter.get() == NULL) {
        return NULL;
    }

    jobject printWriter =
            (*env)->NewObject(e, printWriterClass.get(), printWriterCtor, stringWriter.get());
    if (printWriter == NULL) {
        return NULL;
    }

    scoped_local_ref<jclass> exceptionClass(env, (*env)->GetObjectClass(e, exception)); // can't fail
    jmethodID printStackTraceMethod =
            (*env)->GetMethodID(e, exceptionClass.get(), "printStackTrace", "(Ljava/io/PrintWriter;)V");
    (*env)->CallVoidMethod(e, exception, printStackTraceMethod, printWriter);

    if ((*env)->ExceptionCheck(e)) {
        return NULL;
    }

    scoped_local_ref<jstring> messageStr(env,
            (jstring) (*env)->CallObjectMethod(e, stringWriter.get(), stringWriterToStringMethod));
    if (messageStr.get() == NULL) {
        return NULL;
    }

    const char* utfChars = (*env)->GetStringUTFChars(e, messageStr.get(), NULL);
    if (utfChars == NULL) {
        return NULL;
    }

    char* result = strdup(utfChars);
    (*env)->ReleaseStringUTFChars(e, messageStr.get(), utfChars);
    return result;
}

extern "C" int jniThrowException(C_JNIEnv* env, const char* className, const char* msg) {
    JNIEnv* e = reinterpret_cast<JNIEnv*>(env);

    if ((*env)->ExceptionCheck(e)) {
        /* TODO: consider creating the new exception with this as "cause" */
        scoped_local_ref<jthrowable> exception(env, (*env)->ExceptionOccurred(e));
        (*env)->ExceptionClear(e);

        if (exception.get() != NULL) {
            char* text = getExceptionSummary(env, exception.get());
            ALOGW("Discarding pending exception (%s) to throw %s", text, className);
            free(text);
        }
    }

    scoped_local_ref<jclass> exceptionClass(env, findClass(env, className));
    if (exceptionClass.get() == NULL) {
        ALOGE("Unable to find exception class %s", className);
        /* ClassNotFoundException now pending */
        return -1;
    }

    if ((*env)->ThrowNew(e, exceptionClass.get(), msg) != JNI_OK) {
        ALOGE("Failed throwing '%s' '%s'", className, msg);
        /* an exception, most likely OOM, will now be pending */
        return -1;
    }

    return 0;
}

int jniThrowExceptionFmt(C_JNIEnv* env, const char* className, const char* fmt, va_list args) {
    char msgBuf[512];
    vsnprintf(msgBuf, sizeof(msgBuf), fmt, args);
    return jniThrowException(env, className, msgBuf);
}

int jniThrowNullPointerException(C_JNIEnv* env, const char* msg) {
    return jniThrowException(env, "java/lang/NullPointerException", msg);
}

int jniThrowRuntimeException(C_JNIEnv* env, const char* msg) {
    return jniThrowException(env, "java/lang/RuntimeException", msg);
}

#if 0
int jniThrowIOException(C_JNIEnv* env, int errnum) {
    char buffer[80];
    const char* message = jniStrError(errnum, buffer, sizeof(buffer));
    return jniThrowException(env, "java/io/IOException", message);
}
#endif

void jniLogException(C_JNIEnv* env, int priority, const char* tag, jthrowable exception) {
    JNIEnv* e = reinterpret_cast<JNIEnv*>(env);

    scoped_local_ref<jthrowable> currentException(env, (*env)->ExceptionOccurred(e));
    if (exception == NULL) {
        exception = currentException.get();
        if (exception == NULL) {
            return;
        }
    }

    if (currentException.get() != NULL) {
        (*env)->ExceptionClear(e);
    }

    char* buffer = getStackTrace(env, exception);
    if (buffer == NULL) {
        (*env)->ExceptionClear(e);
        buffer = getExceptionSummary(env, exception);
    }

    __android_log_write(priority, tag, buffer);
    free(buffer);

    if (currentException.get() != NULL) {
        (*env)->Throw(e, currentException.get()); // rethrow
    }
}

#if 0
const char* jniStrError(int errnum, char* buf, size_t buflen) {
    // Note: glibc has a nonstandard strerror_r that returns char* rather than POSIX's int.
    // char *strerror_r(int errnum, char *buf, size_t n);
    char* ret = (char*) strerror_r(errnum, buf, buflen);
    if (((int)ret) == 0) {
        // POSIX strerror_r, success
        return buf;
    } else if (((int)ret) == -1) {
        // POSIX strerror_r, failure
        // (Strictly, POSIX only guarantees a value other than 0. The safest
        // way to implement this function is to use C++ and overload on the
        // type of strerror_r to accurately distinguish GNU from POSIX. But
        // realistic implementations will always return -1.)
        snprintf(buf, buflen, "errno %d", errnum);
        return buf;
    } else {
        // glibc strerror_r returning a string
        return ret;
    }
}
#endif

static struct CachedFields {
    jclass fileDescriptorClass;
    jmethodID fileDescriptorCtor;
    jfieldID descriptorField;
} gCachedFields;

jint JNI_OnLoad(JavaVM* vm, void*) {
    JNIEnv* env;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_4) != JNI_OK) {
        ALOGE("JavaVM::GetEnv() failed");
        abort();
    }

    gCachedFields.fileDescriptorClass =
            reinterpret_cast<jclass>(env->NewGlobalRef(env->FindClass("java/io/FileDescriptor")));
    if (gCachedFields.fileDescriptorClass == NULL) {
        abort();
    }

    gCachedFields.fileDescriptorCtor =
            env->GetMethodID(gCachedFields.fileDescriptorClass, "<init>", "()V");
    if (gCachedFields.fileDescriptorCtor == NULL) {
        abort();
    }

    gCachedFields.descriptorField =
            env->GetFieldID(gCachedFields.fileDescriptorClass, "descriptor", "I");
    if (gCachedFields.descriptorField == NULL) {
        abort();
    }

    return JNI_VERSION_1_4;
}

jobject jniCreateFileDescriptor(C_JNIEnv* env, int fd) {
    JNIEnv* e = reinterpret_cast<JNIEnv*>(env);
    jobject fileDescriptor = (*env)->NewObject(e,
            gCachedFields.fileDescriptorClass, gCachedFields.fileDescriptorCtor);
    jniSetFileDescriptorOfFD(env, fileDescriptor, fd);
    return fileDescriptor;
}

int jniGetFDFromFileDescriptor(C_JNIEnv* env, jobject fileDescriptor) {
    JNIEnv* e = reinterpret_cast<JNIEnv*>(env);
    return (*env)->GetIntField(e, fileDescriptor, gCachedFields.descriptorField);
}

void jniSetFileDescriptorOfFD(C_JNIEnv* env, jobject fileDescriptor, int value) {
    JNIEnv* e = reinterpret_cast<JNIEnv*>(env);
    (*env)->SetIntField(e, fileDescriptor, gCachedFields.descriptorField, value);
}

/*
 * DO NOT USE THIS FUNCTION
 *
 * Get a pointer to the elements of a non-movable array.
 *
 * The semantics are similar to GetDirectBufferAddress.  Specifically, the VM
 * guarantees that the array will not move, and the caller must ensure that
 * it does not continue to use the pointer after the object is collected.
 *
 * We currently use an illegal sequence that trips up CheckJNI when
 * the "forcecopy" mode is enabled.  We pass in a magic value to work
 * around the problem.
 *
 * Returns NULL if the array is movable.
 */
#define kNoCopyMagic 0xd5aab57f     /* also in CheckJni.c */
extern "C" jbyte* jniGetNonMovableArrayElements(C_JNIEnv* env, jarray arrayObj) {
    JNIEnv* e = reinterpret_cast<JNIEnv*>(env);

    jbyteArray byteArray = reinterpret_cast<jbyteArray>(arrayObj);

    /*
     * Normally the "isCopy" parameter is for a return value only, so the
     * non-CheckJNI VM will ignore whatever we pass in.
     */
    uint32_t noCopy = kNoCopyMagic;
    jbyte* result = (*env)->GetByteArrayElements(e, byteArray, reinterpret_cast<jboolean*>(&noCopy));

    /*
     * The non-CheckJNI implementation only cares about the array object,
     * so we can replace the element pointer with the magic value.
     */
    (*env)->ReleaseByteArrayElements(e, byteArray, reinterpret_cast<jbyte*>(kNoCopyMagic), 0);
    return result;
}
