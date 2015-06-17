/*
 * Copyright (C) 2013-2014 Zhang Rui <bbcallen@gmail.com>
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

package tv.danmaku.ijk.media.player;

import android.annotation.TargetApi;
import android.content.Context;
import android.os.Build;
import android.view.Surface;

/**
 * @author bbcallen
 * 
 *         Optional interface default implements
 */
public abstract class BaseMediaPlayer implements IMediaPlayer {
    private boolean mIsLogEnabled;

    public boolean isLogEnabled() {
        return mIsLogEnabled;
    }

    @Override
    public void setLogEnabled(boolean enable) {
        mIsLogEnabled = enable;
    }

    @Override
    public boolean isPlayable() {
        return true;
    }

    @Override
    public void setAudioStreamType(int streamtype) {
    }

    @Override
    public void setKeepInBackground(boolean keepInBackground) {
    }

    @Override
    public int getVideoSarNum() {
        return 1;
    }

    @Override
    public int getVideoSarDen() {
        return 1;
    }

    @Deprecated
    @Override
    public void setWakeMode(Context context, int mode) {
        return;
    }

    @TargetApi(Build.VERSION_CODES.JELLY_BEAN_MR1)
    @Override
    public void setSurface(Surface surface) {
    }
}
