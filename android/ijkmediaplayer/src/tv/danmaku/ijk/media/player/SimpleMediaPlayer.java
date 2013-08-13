/*
 * Copyright (C) 2006 The Android Open Source Project
 * Copyright (C) 2013 Zhang Rui <bbcallen@gmail.com>
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

public abstract class SimpleMediaPlayer extends AbstractMediaPlayer {
    private OnPreparedListener mOnPreparedListener;
    private OnCompletionListener mOnCompletionListener;
    private OnBufferingUpdateListener mOnBufferingUpdateListener;
    private OnSeekCompleteListener mOnSeekCompleteListener;
    private OnVideoSizeChangedListener mOnVideoSizeChangedListener;
    private OnErrorListener mOnErrorListener;
    private OnInfoListener mOnInfoListener;

    public final void setOnPreparedListener(OnPreparedListener listener) {
        mOnPreparedListener = listener;
    }

    public final void setOnCompletionListener(OnCompletionListener listener) {
        mOnCompletionListener = listener;
    }

    public final void setOnBufferingUpdateListener(
            OnBufferingUpdateListener listener) {
        mOnBufferingUpdateListener = listener;
    }

    public final void setOnSeekCompleteListener(OnSeekCompleteListener listener) {
        mOnSeekCompleteListener = listener;
    }

    public final void setOnVideoSizeChangedListener(
            OnVideoSizeChangedListener listener) {
        mOnVideoSizeChangedListener = listener;
    }

    public final void setOnErrorListener(OnErrorListener listener) {
        mOnErrorListener = listener;
    }

    public final void setOnInfoListener(OnInfoListener listener) {
        mOnInfoListener = listener;
    }

    public final void resetListeners() {
        mOnPreparedListener = null;
        mOnBufferingUpdateListener = null;
        mOnCompletionListener = null;
        mOnSeekCompleteListener = null;
        mOnVideoSizeChangedListener = null;
        mOnErrorListener = null;
        mOnInfoListener = null;
    }

    public void attachListeners(AbstractMediaPlayer mp) {
        mp.setOnPreparedListener(mOnPreparedListener);
        mp.setOnBufferingUpdateListener(mOnBufferingUpdateListener);
        mp.setOnCompletionListener(mOnCompletionListener);
        mp.setOnSeekCompleteListener(mOnSeekCompleteListener);
        mp.setOnVideoSizeChangedListener(mOnVideoSizeChangedListener);
        mp.setOnErrorListener(mOnErrorListener);
        mp.setOnInfoListener(mOnInfoListener);
    }

    protected static final void notifyOnPrepared(SimpleMediaPlayer mp) {
        if (mp != null && mp.mOnPreparedListener != null)
            mp.mOnPreparedListener.onPrepared(mp);
    }

    protected static final void notifyOnCompletion(SimpleMediaPlayer mp) {
        if (mp != null && mp.mOnCompletionListener != null)
            mp.mOnCompletionListener.onCompletion(mp);
    }

    protected static final void notifyOnBufferingUpdate(SimpleMediaPlayer mp,
            int percent) {
        if (mp != null && mp.mOnBufferingUpdateListener != null)
            mp.mOnBufferingUpdateListener.onBufferingUpdate(mp, percent);
    }

    protected static final void notifyOnSeekComplete(SimpleMediaPlayer mp) {
        if (mp != null && mp.mOnSeekCompleteListener != null)
            mp.mOnSeekCompleteListener.onSeekComplete(mp);
    }

    protected static final void notifyOnVideoSizeChanged(SimpleMediaPlayer mp,
            int width, int height, int sarNum, int sarDen) {
        if (mp != null && mp.mOnVideoSizeChangedListener != null)
            mp.mOnVideoSizeChangedListener.onVideoSizeChanged(mp, width,
                    height, sarNum, sarDen);
    }

    protected static final boolean notifyOnError(SimpleMediaPlayer mp,
            int what, int extra) {
        if (mp != null && mp.mOnErrorListener != null)
            return mp.mOnErrorListener.onError(mp, what, extra);
        return false;
    }

    protected static final boolean notifyOnInfo(SimpleMediaPlayer mp, int what,
            int extra) {
        if (mp != null && mp.mOnInfoListener != null)
            return mp.mOnInfoListener.onInfo(mp, what, extra);
        return false;
    }
}
