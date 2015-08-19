/*
 * Copyright (C) 2015 Zhang Rui <bbcallen@gmail.com>
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

package tv.danmaku.ijk.media.sample.application;

import android.content.Context;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;

import tv.danmaku.ijk.media.sample.R;

public class Settings {
    private Context mAppContext;
    private SharedPreferences mSharedPreferences;

    public Settings(Context context) {
        mAppContext = context.getApplicationContext();
        mSharedPreferences = PreferenceManager.getDefaultSharedPreferences(mAppContext);
    }

    public boolean getUsingAndroidPlayer() {
        String key = mAppContext.getString(R.string.pref_key_using_android_player);
        return mSharedPreferences.getBoolean(key, false);
    }

    public boolean getUsingTextureView() {
        String key = mAppContext.getString(R.string.pref_key_using_texture_view);
        return mSharedPreferences.getBoolean(key, false);
    }

    public boolean getUsingMediaCodec() {
        String key = mAppContext.getString(R.string.pref_key_using_media_codec);
        return mSharedPreferences.getBoolean(key, false);
    }

    public boolean getUsingOpenSLES() {
        String key = mAppContext.getString(R.string.pref_key_using_opensl_es);
        return mSharedPreferences.getBoolean(key, false);
    }

    public String getPixelFormat() {
        String key = mAppContext.getString(R.string.pref_key_pixel_format);
        return mSharedPreferences.getString(key, "");
    }
}
