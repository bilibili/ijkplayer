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
import java.lang.ref.WeakReference;

import android.content.Context;
import android.media.MediaPlayer;
import android.view.Surface;
import android.view.SurfaceHolder;

public final class AndroidMediaPlayer extends SimpleMediaPlayer {
    private MediaPlayer mInternalMediaPlayer;
    private MediaPlayerListenerAdapter mInternalListenerAdapter;

    public AndroidMediaPlayer() {
        mInternalMediaPlayer = new MediaPlayer();
        mInternalListenerAdapter = new MediaPlayerListenerAdapter(this);
        attachInternalListeners();
    }

    @Override
    public void setDisplay(SurfaceHolder sh) {
        mInternalMediaPlayer.setDisplay(sh);
    }

    @Override
    public void setSurface(Surface surface) {
        mInternalMediaPlayer.setSurface(surface);
    }

    @Override
    public void setDataSource(String path) throws IOException,
            IllegalArgumentException, SecurityException, IllegalStateException {
        mInternalMediaPlayer.setDataSource(path);
    }

    @Override
    public void prepareAsync() throws IllegalStateException {
        mInternalMediaPlayer.prepareAsync();
    }

    @Override
    public void start() throws IllegalStateException {
        mInternalMediaPlayer.start();
    }

    @Override
    public void stop() throws IllegalStateException {
        mInternalMediaPlayer.stop();
    }

    @Override
    public void pause() throws IllegalStateException {
        mInternalMediaPlayer.pause();
    }

    @Override
    public void setWakeMode(Context context, int mode) {
        mInternalMediaPlayer.setWakeMode(context, mode);
    }

    @Override
    public void setScreenOnWhilePlaying(boolean screenOn) {
        mInternalMediaPlayer.setScreenOnWhilePlaying(screenOn);
    }

    @Override
    public int getVideoWidth() {
        return mInternalMediaPlayer.getVideoWidth();
    }

    @Override
    public int getVideoHeight() {
        return mInternalMediaPlayer.getVideoHeight();
    }

    @Override
    public boolean isPlaying() {
        return mInternalMediaPlayer.isPlaying();
    }

    @Override
    public void seekTo(long msec) throws IllegalStateException {
        mInternalMediaPlayer.seekTo((int) msec);
    }

    @Override
    public long getCurrentPosition() {
        return mInternalMediaPlayer.getCurrentPosition();
    }

    @Override
    public long getDuration() {
        return mInternalMediaPlayer.getDuration();
    }

    @Override
    public void release() {
        mInternalMediaPlayer.release();

        resetListeners();
        attachInternalListeners();
    }

    @Override
    public void reset() {
        mInternalMediaPlayer.reset();

        resetListeners();
        attachInternalListeners();
    }

    @Override
    public void setAudioStreamType(int streamtype) {
        mInternalMediaPlayer.setAudioStreamType(streamtype);
    }

    /*--------------------
     * Listeners adapter
     */
    private final void attachInternalListeners() {
        mInternalMediaPlayer.setOnPreparedListener(mInternalListenerAdapter);
        mInternalMediaPlayer
                .setOnBufferingUpdateListener(mInternalListenerAdapter);
        mInternalMediaPlayer.setOnCompletionListener(mInternalListenerAdapter);
        mInternalMediaPlayer
                .setOnSeekCompleteListener(mInternalListenerAdapter);
        mInternalMediaPlayer
                .setOnVideoSizeChangedListener(mInternalListenerAdapter);
        mInternalMediaPlayer.setOnErrorListener(mInternalListenerAdapter);
        mInternalMediaPlayer.setOnInfoListener(mInternalListenerAdapter);
    }

    private static class MediaPlayerListenerAdapter implements
            MediaPlayer.OnPreparedListener, MediaPlayer.OnCompletionListener,
            MediaPlayer.OnBufferingUpdateListener,
            MediaPlayer.OnSeekCompleteListener,
            MediaPlayer.OnVideoSizeChangedListener,
            MediaPlayer.OnErrorListener, MediaPlayer.OnInfoListener {
        public WeakReference<AndroidMediaPlayer> mWeakMediaPlayer;

        public MediaPlayerListenerAdapter(AndroidMediaPlayer mp) {
            mWeakMediaPlayer = new WeakReference<AndroidMediaPlayer>(mp);
        }

        @Override
        public boolean onInfo(MediaPlayer mp, int what, int extra) {
            return notifyOnInfo(mWeakMediaPlayer.get(), what, extra);
        }

        @Override
        public boolean onError(MediaPlayer mp, int what, int extra) {
            return notifyOnError(mWeakMediaPlayer.get(), what, extra);
        }

        @Override
        public void onVideoSizeChanged(MediaPlayer mp, int width, int height) {
            notifyOnVideoSizeChanged(mWeakMediaPlayer.get(), width, height, 1,
                    1);
        }

        @Override
        public void onSeekComplete(MediaPlayer mp) {
            notifyOnSeekComplete(mWeakMediaPlayer.get());
        }

        @Override
        public void onBufferingUpdate(MediaPlayer mp, int percent) {
            notifyOnBufferingUpdate(mWeakMediaPlayer.get(), percent);
        }

        @Override
        public void onCompletion(MediaPlayer mp) {
            notifyOnCompletion(mWeakMediaPlayer.get());
        }

        @Override
        public void onPrepared(MediaPlayer mp) {
            notifyOnPrepared(mWeakMediaPlayer.get());
        }
    }
}
