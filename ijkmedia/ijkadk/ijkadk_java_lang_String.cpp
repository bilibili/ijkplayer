/*****************************************************************************
 * ijkadk_java_lang_String.cpp
 *****************************************************************************
 *
 * copyright (c) 2013-2014 Zhang Rui <bbcallen@gmail.com>
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

#include "ijkadk_java_lang_String.hpp"

using namespace ::ijkadk;
using namespace ::ijkadk::java::lang;

typedef struct StringClass
{
    jclass      clazz;
} StringClass;
static StringClass gClazz;

int String::loadClass(JNIEnv *env)
{
    ADKJniClassLoadHelper helper(env);
    gClazz.clazz = helper.findClassAsGlobalRef("Ljava/lang/String;");

    return helper.getIntResult();
}

String::~String()
{
    jstring thiz = getThiz();
    if (!thiz)
        return;

    JNIEnv *env = getJNIEnv();
    if (mStringUTFChars) {
        env->ReleaseStringUTFChars(thiz, mStringUTFChars);
        mStringUTFChars = NULL;
    }
}

ADKPtr<String> String::createWithUTFChars(const char *utfChars)
{
    JNIEnv *env = getJNIEnv();
    if (utfChars == NULL)
        return create();

    ADKLocalRef<jstring> jString(env->NewStringUTF(utfChars));
    IJKADK_VALIDATE(jString);

    return create(jString.get());
}

const char *String::getUTFChars()
{
    if (mStringUTFChars)
        return mStringUTFChars;

    jstring thiz = getThiz();
    if (!thiz)
        return NULL;

    JNIEnv *env = getJNIEnv();
    mStringUTFChars = env->GetStringUTFChars(thiz, NULL);

    return mStringUTFChars;
}
