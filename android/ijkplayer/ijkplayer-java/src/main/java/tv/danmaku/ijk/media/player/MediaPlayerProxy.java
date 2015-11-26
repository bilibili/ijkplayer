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
import android.content.Context;
import android.net.Uri;
import android.os.Build;
import android.view.Surface;
import android.view.SurfaceHolder;

import java.io.FileDescriptor;
import java.io.IOException;
import java.util.Map;

import tv.danmaku.ijk.media.player.misc.IMediaDataSource;
import tv.danmaku.ijk.media.player.misc.ITrackInfo;

public class MediaPlayerProxy implements IMediaPlayer {
    protected final IMediaPlayer mBackEndMediaPlayer;

    public MediaPlayerProxy(IMediaPlayer backEndMediaPlayer) {
        mBackEndMediaPlayer = backEndMediaPlayer;
    }

    public IMediaPlayer getInternalMediaPlayer() {
        return mBackEndMediaPlayer;
    }

    @Override
    public void setDisplay(SurfaceHolder sh) {
        mBackEndMediaPlayer.setDisplay(sh);
    }

    @TargetApi(Build.VERSION_CODES.ICE_CREAM_SANDWICH)
    @Override
    public void setSurface(Surface surface) {
        mBackEndMediaPlayer.setSurface(surface);
    }

    @Override
    public void setDataSource(Context context, Uri uri)
            throws IOException, IllegalArgumentException, SecurityException, IllegalStateException {
        mBackEndMediaPlayer.setDataSource(context, uri);
    }

    @TargetApi(Build.VERSION_CODES.ICE_CREAM_SANDWICH)
    @Override
    public void setDataSource(Context context, Uri uri, Map<String, String> headers)
            throws IOException, IllegalArgumentException, SecurityException, IllegalStateException {
        mBackEndMediaPlayer.setDataSource(context, uri, headers);
    }

    @Override
    public void setDataSource(FileDescriptor fd)
            throws IOException, IllegalArgumentException, IllegalStateException {
        mBackEndMediaPlayer.setDataSource(fd);
    }

    @Override
    public void setDataSource(String path) throws IOException, IllegalArgumentException, SecurityException, IllegalStateException {
        mBackEndMediaPlayer.setDataSource(path);
    }

    @Override
    public void setDataSource(IMediaDataSource mediaDataSource)  {
        mBackEndMediaPlayer.setDataSource(mediaDataSource);
    }

    @Override
    public String getDataSource() {
        return mBackEndMediaPlayer.getDataSource();
    }

    @Override
    public void prepareAsync() throws IllegalStateException {
        mBackEndMediaPlayer.prepareAsync();
    }

    @Override
    public void start() throws IllegalStateException {
        mBackEndMediaPlayer.start();
    }

    @Override
    public void stop() throws IllegalStateException {
        mBackEndMediaPlayer.stop();
    }

    @Override
    public void pause() throws IllegalStateException {
        mBackEndMediaPlayer.pause();
    }

    @Override
    public void setScreenOnWhilePlaying(boolean screenOn) {
        mBackEndMediaPlayer.setScreenOnWhilePlaying(screenOn);
    }

    @Override
    public int getVideoWidth() {
        return mBackEndMediaPlayer.getVideoWidth();
    }

    @Override
    public int getVideoHeight() {
        return mBackEndMediaPlayer.getVideoHeight();
    }

    @Override
    public boolean isPlaying() {
        return mBackEndMediaPlayer.isPlaying();
    }

    @Override
    public void seekTo(long msec) throws IllegalStateException {
        mBackEndMediaPlayer.seekTo(msec);
    }

    @Override
    public long getCurrentPosition() {
        return mBackEndMediaPlayer.getCurrentPosition();
    }

    @Override
    public long getDuration() {
        return mBackEndMediaPlayer.getDuration();
    }

    @Override
    public void release() {
        mBackEndMediaPlayer.release();
    }

    @Override
    public void reset() {
        mBackEndMediaPlayer.reset();
    }

    @Override
    public void setVolume(float leftVolume, float rightVolume) {
        mBackEndMediaPlayer.setVolume(leftVolume, rightVolume);
    }

    @Override
    public int getAudioSessionId() {
        return mBackEndMediaPlayer.getAudioSessionId();
    }

    @Override
    public MediaInfo getMediaInfo() {
        return mBackEndMediaPlayer.getMediaInfo();
    }

    @Override
    public void setLogEnabled(boolean enable) {

    }

    @Override
    public boolean isPlayable() {
        return false;
    }

    @Override
    public void setOnPreparedListener(OnPreparedListener listener) {
        if (listener != null) {
            final OnPreparedListener finalListener = listener;
            mBackEndMediaPlayer.setOnPreparedListener(new OnPreparedListener() {
                @Override
                public void onPrepared(IMediaPlayer mp) {
                    finalListener.onPrepared(MediaPlayerProxy.this);
                }
            });
        } else {
            mBackEndMediaPlayer.setOnPreparedListener(null);
        }
    }

    @Override
    public void setOnCompletionListener(OnCompletionListener listener) {
        if (listener != null) {
            final OnCompletionListener finalListener = listener;
            mBackEndMediaPlayer.setOnCompletionListener(new OnCompletionListener() {
                @Override
                public void onCompletion(IMediaPlayer mp) {
                    finalListener.onCompletion(MediaPlayerProxy.this);
                }
            });
        } else {
            mBackEndMediaPlayer.setOnCompletionListener(null);
        }
    }

    @Override
    public void setOnBufferingUpdateListener(OnBufferingUpdateListener listener) {
        if (listener != null) {
            final OnBufferingUpdateListener finalListener = listener;
            mBackEndMediaPlayer.setOnBufferingUpdateListener(new OnBufferingUpdateListener() {
                @Override
                public void onBufferingUpdate(IMediaPlayer mp, int percent) {
                    finalListener.onBufferingUpdate(MediaPlayerProxy.this, percent);
                }
            });
        } else {
            mBackEndMediaPlayer.setOnBufferingUpdateListener(null);
        }
    }

    @Override
    public void setOnSeekCompleteListener(OnSeekCompleteListener listener) {
        if (listener != null) {
            final OnSeekCompleteListener finalListener = listener;
            mBackEndMediaPlayer.setOnSeekCompleteListener(new OnSeekCompleteListener() {
                @Override
                public void onSeekComplete(IMediaPlayer mp) {
                    finalListener.onSeekComplete(MediaPlayerProxy.this);
                }
            });
        } else {
            mBackEndMediaPlayer.setOnSeekCompleteListener(null);
        }
    }

    @Override
    public void setOnVideoSizeChangedListener(OnVideoSizeChangedListener listener) {
        if (listener != null) {
            final OnVideoSizeChangedListener finalListener = listener;
            mBackEndMediaPlayer.setOnVideoSizeChangedListener(new OnVideoSizeChangedListener() {
                @Override
                public void onVideoSizeChanged(IMediaPlayer mp, int width, int height, int sar_num, int sar_den) {
                    finalListener.onVideoSizeChanged(MediaPlayerProxy.this, width, height, sar_num, sar_den);
                }
            });
        } else {
            mBackEndMediaPlayer.setOnVideoSizeChangedListener(null);
        }
    }

    @Override
    public void setOnErrorListener(OnErrorListener listener) {
        if (listener != null) {
            final OnErrorListener finalListener = listener;
            mBackEndMediaPlayer.setOnErrorListener(new OnErrorListener() {
                @Override
                public boolean onError(IMediaPlayer mp, int what, int extra) {
                    return finalListener.onError(MediaPlayerProxy.this, what, extra);
                }
            });
        } else {
            mBackEndMediaPlayer.setOnErrorListener(null);
        }
    }

    @Override
    public void setOnInfoListener(OnInfoListener listener) {
        if (listener != null) {
            final OnInfoListener finalListener = listener;
            mBackEndMediaPlayer.setOnInfoListener(new OnInfoListener() {
                @Override
                public boolean onInfo(IMediaPlayer mp, int what, int extra) {
                    return finalListener.onInfo(MediaPlayerProxy.this, what, extra);
                }
            });
        } else {
            mBackEndMediaPlayer.setOnInfoListener(null);
        }
    }

    @Override
    public void setAudioStreamType(int streamtype) {
        mBackEndMediaPlayer.setAudioStreamType(streamtype);
    }

    @Override
    public void setKeepInBackground(boolean keepInBackground) {
        mBackEndMediaPlayer.setKeepInBackground(keepInBackground);
    }

    @Override
    public int getVideoSarNum() {
        return mBackEndMediaPlayer.getVideoSarNum();
    }

    @Override
    public int getVideoSarDen() {
        return mBackEndMediaPlayer.getVideoSarDen();
    }

    @Override
    public void setWakeMode(Context context, int mode) {
        mBackEndMediaPlayer.setWakeMode(context, mode);
    }

    @Override
    public ITrackInfo[] getTrackInfo() {
        return mBackEndMediaPlayer.getTrackInfo();
    }

    @Override
    public void setLooping(boolean looping) {
        mBackEndMediaPlayer.setLooping(looping);
    }

    @Override
    public boolean isLooping() {
        return mBackEndMediaPlayer.isLooping();
    }
}
