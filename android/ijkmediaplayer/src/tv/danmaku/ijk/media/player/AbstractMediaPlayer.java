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

import java.io.IOException;

import android.content.Context;
import android.view.Surface;
import android.view.SurfaceHolder;

public abstract class AbstractMediaPlayer {
    /*
     * Do not change these values without updating their counterparts in native
     */
    protected static final int MEDIA_NOP = 0; // interface test message
    protected static final int MEDIA_PREPARED = 1;
    protected static final int MEDIA_PLAYBACK_COMPLETE = 2;
    protected static final int MEDIA_BUFFERING_UPDATE = 3;
    protected static final int MEDIA_SEEK_COMPLETE = 4;
    protected static final int MEDIA_SET_VIDEO_SIZE = 5;
    protected static final int MEDIA_TIMED_TEXT = 99;
    protected static final int MEDIA_ERROR = 100;
    protected static final int MEDIA_INFO = 200;

    protected static final int MEDIA_SET_VIDEO_SAR = 10001;

    public static final int MEDIA_INFO_UNKNOWN = 1;
    public static final int MEDIA_INFO_STARTED_AS_NEXT = 2;
    public static final int MEDIA_INFO_VIDEO_RENDERING_START = 3;
    public static final int MEDIA_INFO_VIDEO_TRACK_LAGGING = 700;
    public static final int MEDIA_INFO_BUFFERING_START = 701;
    public static final int MEDIA_INFO_BUFFERING_END = 702;
    public static final int MEDIA_INFO_BAD_INTERLEAVING = 800;
    public static final int MEDIA_INFO_NOT_SEEKABLE = 801;
    public static final int MEDIA_INFO_METADATA_UPDATE = 802;
    public static final int MEDIA_INFO_TIMED_TEXT_ERROR = 900;

    public static final int MEDIA_ERROR_UNKNOWN = 1;
    public static final int MEDIA_ERROR_SERVER_DIED = 100;
    public static final int MEDIA_ERROR_NOT_VALID_FOR_PROGRESSIVE_PLAYBACK = 200;
    public static final int MEDIA_ERROR_IO = -1004;
    public static final int MEDIA_ERROR_MALFORMED = -1007;
    public static final int MEDIA_ERROR_UNSUPPORTED = -1010;
    public static final int MEDIA_ERROR_TIMED_OUT = -110;

    public abstract void setDisplay(SurfaceHolder sh);

    public abstract void setSurface(Surface surface);

    public abstract void setDataSource(String path) throws IOException,
            IllegalArgumentException, SecurityException, IllegalStateException;

    public abstract void prepareAsync() throws IllegalStateException;

    public abstract void start() throws IllegalStateException;

    public abstract void stop() throws IllegalStateException;

    public abstract void pause() throws IllegalStateException;

    public abstract void setWakeMode(Context context, int mode);

    public abstract void setScreenOnWhilePlaying(boolean screenOn);

    public abstract int getVideoWidth();

    public abstract int getVideoHeight();

    public abstract boolean isPlaying();

    public abstract void seekTo(long msec) throws IllegalStateException;

    public abstract long getCurrentPosition();

    public abstract long getDuration();

    public abstract void release();

    public abstract void reset();

    public abstract void setAudioStreamType(int streamtype);

    /*--------------------
     * Listeners
     */
    public interface OnPreparedListener {
        void onPrepared(AbstractMediaPlayer mp);
    }

    public interface OnCompletionListener {
        void onCompletion(AbstractMediaPlayer mp);
    }

    public interface OnBufferingUpdateListener {
        void onBufferingUpdate(AbstractMediaPlayer mp, int percent);
    }

    public interface OnSeekCompleteListener {
        public void onSeekComplete(AbstractMediaPlayer mp);
    }

    public interface OnVideoSizeChangedListener {
        public void onVideoSizeChanged(AbstractMediaPlayer mp, int width,
                int height, int sar_num, int sar_den);
    }

    public interface OnErrorListener {
        boolean onError(AbstractMediaPlayer mp, int what, int extra);
    }

    public interface OnInfoListener {
        boolean onInfo(AbstractMediaPlayer mp, int what, int extra);
    }

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

    protected static final void notifyOnPrepared(AbstractMediaPlayer mp) {
        if (mp != null && mp.mOnPreparedListener != null)
            mp.mOnPreparedListener.onPrepared(mp);
    }

    protected static final void notifyOnCompletion(AbstractMediaPlayer mp) {
        if (mp != null && mp.mOnCompletionListener != null)
            mp.mOnCompletionListener.onCompletion(mp);
    }

    protected static final void notifyOnBufferingUpdate(AbstractMediaPlayer mp,
            int percent) {
        if (mp != null && mp.mOnBufferingUpdateListener != null)
            mp.mOnBufferingUpdateListener.onBufferingUpdate(mp, percent);
    }

    protected static final void notifyOnSeekComplete(AbstractMediaPlayer mp) {
        if (mp != null && mp.mOnSeekCompleteListener != null)
            mp.mOnSeekCompleteListener.onSeekComplete(mp);
    }

    protected static final void notifyOnVideoSizeChanged(
            AbstractMediaPlayer mp, int width, int height, int sarNum,
            int sarDen) {
        if (mp != null && mp.mOnVideoSizeChangedListener != null)
            mp.mOnVideoSizeChangedListener.onVideoSizeChanged(mp, width,
                    height, sarNum, sarDen);
    }

    protected static final boolean notifyOnError(AbstractMediaPlayer mp,
            int what, int extra) {
        if (mp != null && mp.mOnErrorListener != null)
            return mp.mOnErrorListener.onError(mp, what, extra);
        return false;
    }

    protected static final boolean notifyOnInfo(AbstractMediaPlayer mp,
            int what, int extra) {
        if (mp != null && mp.mOnInfoListener != null)
            return mp.mOnInfoListener.onInfo(mp, what, extra);
        return false;
    }
}
