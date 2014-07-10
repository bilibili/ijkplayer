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

using namespace ::ijkadk::java::lang;

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

String* String::createWithUTFChars(const char *utfChars)
{
    JNIEnv *env = getJNIEnv();
    ADKPtr<String> str(create());
    if (utfChars) {
        jstring jString = env->NewStringUTF(utfChars);
        IJKADK_VALIDATE(jString);

        str->init(jString);

        env->DeleteLocalRef(jString);
    }
    return str.detach();
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
