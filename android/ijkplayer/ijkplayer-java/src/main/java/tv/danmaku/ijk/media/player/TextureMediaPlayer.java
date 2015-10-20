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

package tv.danmaku.ijk.media.player;

import android.annotation.TargetApi;
import android.graphics.SurfaceTexture;
import android.os.Build;
import android.view.Surface;
import android.view.SurfaceHolder;

@TargetApi(Build.VERSION_CODES.ICE_CREAM_SANDWICH)
public class TextureMediaPlayer extends MediaPlayerProxy implements IMediaPlayer, ISurfaceTextureHolder {
    private SurfaceTexture mSurfaceTexture;
    private ISurfaceTextureHost mSurfaceTextureHost;

    public TextureMediaPlayer(IMediaPlayer backEndMediaPlayer) {
        super(backEndMediaPlayer);
    }

    public void releaseSurfaceTexture() {
        if (mSurfaceTexture != null) {
            if (mSurfaceTextureHost != null) {
                mSurfaceTextureHost.releaseSurfaceTexture(mSurfaceTexture);
            } else {
                mSurfaceTexture.release();
            }
            mSurfaceTexture = null;
        }
    }

    //--------------------
    // IMediaPlayer
    //--------------------
    @Override
    public void reset() {
        super.reset();
        releaseSurfaceTexture();
    }

    @Override
    public void release() {
        super.release();
        releaseSurfaceTexture();
    }

    @Override
    public void setDisplay(SurfaceHolder sh) {
        if (mSurfaceTexture == null)
            super.setDisplay(sh);
    }

    @Override
    public void setSurface(Surface surface) {
        if (mSurfaceTexture == null)
            super.setSurface(surface);
    }

    //--------------------
    // ISurfaceTextureHolder
    //--------------------

    @Override
    public void setSurfaceTexture(SurfaceTexture surfaceTexture) {
        if (mSurfaceTexture == surfaceTexture)
            return;

        releaseSurfaceTexture();
        mSurfaceTexture = surfaceTexture;
        if (surfaceTexture == null) {
            super.setSurface(null);
        } else {
            super.setSurface(new Surface(surfaceTexture));
        }
    }

    @Override
    public SurfaceTexture getSurfaceTexture() {
        return mSurfaceTexture;
    }

    @Override
    public void setSurfaceTextureHost(ISurfaceTextureHost surfaceTextureHost) {
        mSurfaceTextureHost = surfaceTextureHost;
    }
}
