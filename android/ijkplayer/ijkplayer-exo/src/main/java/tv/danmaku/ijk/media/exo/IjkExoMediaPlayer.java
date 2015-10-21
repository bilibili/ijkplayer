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

package tv.danmaku.ijk.media.exo;

import android.content.Context;
import android.net.Uri;
import android.view.Surface;
import android.view.SurfaceHolder;

import com.google.android.exoplayer.ExoPlayer;
import com.google.android.exoplayer.util.Util;

import java.io.FileDescriptor;
import java.util.Map;

import tv.danmaku.ijk.media.exo.demo.EventLogger;
import tv.danmaku.ijk.media.exo.demo.player.DemoPlayer;
import tv.danmaku.ijk.media.exo.demo.player.DemoPlayer.RendererBuilder;
import tv.danmaku.ijk.media.exo.demo.player.ExtractorRendererBuilder;
import tv.danmaku.ijk.media.player.AbstractMediaPlayer;
import tv.danmaku.ijk.media.player.IMediaPlayer;
import tv.danmaku.ijk.media.player.MediaInfo;
import tv.danmaku.ijk.media.player.misc.IjkTrackInfo;

public class IjkExoMediaPlayer extends AbstractMediaPlayer {
    private Context mAppContext;
    private DemoPlayer mInternalPlayer;
    private EventLogger mEventLogger;
    private String mDataSource;
    private int mVideoWidth;
    private int mVideoHeight;
    private Surface mSurface;

    public static final int TYPE_DASH = 0;
    public static final int TYPE_SS = 1;
    public static final int TYPE_HLS = 2;
    public static final int TYPE_OTHER = 3;

    private RendererBuilder mRendererBuilder;

    public IjkExoMediaPlayer(Context context) {
        mAppContext = context.getApplicationContext();

        mDemoListener = new DemoPlayerListener();

        mEventLogger = new EventLogger();
        mEventLogger.startSession();
    }

    @Override
    public void setDisplay(SurfaceHolder sh) {
        if (sh == null)
            setSurface(null);
        else
            setSurface(sh.getSurface());
    }

    @Override
    public void setSurface(Surface surface) {
        mSurface = surface;
        if (mInternalPlayer != null)
            mInternalPlayer.setSurface(surface);
    }

    @Override
    public void setDataSource(Context context, Uri uri) {
        mDataSource = uri.toString();
        mRendererBuilder = new ExtractorRendererBuilder(context, getUserAgent(), uri);
    }

    @Override
    public void setDataSource(Context context, Uri uri, Map<String, String> headers) {
        // TODO: handle headers
        setDataSource(context, uri);
    }

    @Override
    public void setDataSource(String path) {
        setDataSource(mAppContext, Uri.parse(path));
    }

    @Override
    public void setDataSource(FileDescriptor fd) {
        // TODO: no support
        throw new UnsupportedOperationException("no support");
    }

    @Override
    public String getDataSource() {
        return mDataSource;
    }

    @Override
    public void prepareAsync() throws IllegalStateException {
        if (mInternalPlayer != null)
            throw new IllegalStateException("can't prepare a prepared player");

        mInternalPlayer = new DemoPlayer(mRendererBuilder);
        mInternalPlayer.addListener(mDemoListener);
        mInternalPlayer.addListener(mEventLogger);
        mInternalPlayer.setInfoListener(mEventLogger);
        mInternalPlayer.setInternalErrorListener(mEventLogger);

        if (mSurface != null)
            mInternalPlayer.setSurface(mSurface);

        mInternalPlayer.prepare();
        mInternalPlayer.setPlayWhenReady(false);
    }

    @Override
    public void start() throws IllegalStateException {
        if (mInternalPlayer == null)
            return;
        mInternalPlayer.setPlayWhenReady(true);
    }

    @Override
    public void stop() throws IllegalStateException {
        if (mInternalPlayer == null)
            return;
        mInternalPlayer.release();
    }

    @Override
    public void pause() throws IllegalStateException {
        if (mInternalPlayer == null)
            return;
        mInternalPlayer.setPlayWhenReady(false);
    }

    @Override
    public void setWakeMode(Context context, int mode) {
        // FIXME: implement
    }

    @Override
    public void setScreenOnWhilePlaying(boolean screenOn) {
        // TODO: do nothing
    }

    @Override
    public IjkTrackInfo[] getTrackInfo() {
        // TODO: implement
        return null;
    }

    @Override
    public int getVideoWidth() {
        return mVideoWidth;
    }

    @Override
    public int getVideoHeight() {
        return mVideoHeight;
    }

    @Override
    public boolean isPlaying() {
        if (mInternalPlayer == null)
            return false;
        int state = mInternalPlayer.getPlaybackState();
        switch (state) {
            case ExoPlayer.STATE_BUFFERING:
            case ExoPlayer.STATE_READY:
                return mInternalPlayer.getPlayWhenReady();
            case ExoPlayer.STATE_IDLE:
            case ExoPlayer.STATE_PREPARING:
            case ExoPlayer.STATE_ENDED:
                default:
                return false;
        }
    }

    @Override
    public void seekTo(long msec) throws IllegalStateException {
        if (mInternalPlayer == null)
            return;
        mInternalPlayer.seekTo(msec);
    }

    @Override
    public long getCurrentPosition() {
        if (mInternalPlayer == null)
            return 0;
        return mInternalPlayer.getCurrentPosition();
    }

    @Override
    public long getDuration() {
        if (mInternalPlayer == null)
            return 0;
        return mInternalPlayer.getDuration();
    }

    @Override
    public int getVideoSarNum() {
        return 1;
    }

    @Override
    public int getVideoSarDen() {
        return 1;
    }

    @Override
    public void reset() {
        if (mInternalPlayer != null) {
            mInternalPlayer.release();
            mInternalPlayer.removeListener(mDemoListener);
            mInternalPlayer.removeListener(mEventLogger);
            mInternalPlayer.setInfoListener(null);
            mInternalPlayer.setInternalErrorListener(null);
            mInternalPlayer = null;
        }

        mSurface = null;
        mDataSource = null;
        mVideoWidth = 0;
        mVideoHeight = 0;
    }

    @Override
    public void setLooping(boolean looping) {
        // TODO: no support
        throw new UnsupportedOperationException("no support");
    }

    @Override
    public boolean isLooping() {
        // TODO: no support
        return false;
    }

    @Override
    public void setVolume(float leftVolume, float rightVolume) {
        // TODO: no support
    }


    @Override
    public int getAudioSessionId() {
        // TODO: no support
        return 0;
    }

    @Override
    public MediaInfo getMediaInfo() {
        // TODO: no support
        return null;
    }

    @Override
    public void setLogEnabled(boolean enable) {
        // do nothing
    }

    @Override
    public boolean isPlayable() {
        return true;
    }

    @Override
    public void setAudioStreamType(int streamtype) {
        // do nothing
    }

    @Override
    public void setKeepInBackground(boolean keepInBackground) {
        // do nothing
    }

    @Override
    public void release() {
        if (mInternalPlayer != null) {
            reset();

            mDemoListener = null;

            mEventLogger.endSession();
            mEventLogger = null;
        }
    }

    private String getUserAgent() {
        return Util.getUserAgent(mAppContext, "IjkExoMediaPlayer");
    }

    private class DemoPlayerListener implements DemoPlayer.Listener {
        private boolean mIsPrepareing = false;
        private boolean mDidPrepare = false;
        private boolean mIsBuffering = false;

        public void onStateChanged(boolean playWhenReady, int playbackState)
        {
            if (mIsBuffering) {
                switch (playbackState) {
                    case ExoPlayer.STATE_ENDED:
                    case ExoPlayer.STATE_READY:
                        notifyOnInfo(IMediaPlayer.MEDIA_INFO_BUFFERING_END, mInternalPlayer.getBufferedPercentage());
                        mIsBuffering = false;
                        break;
                }
            }

            if (mIsPrepareing) {
                switch (playbackState) {
                    case ExoPlayer.STATE_READY:
                        notifyOnPrepared();
                        mIsPrepareing = false;
                        mDidPrepare = false;
                        break;
                }
            }

            switch (playbackState) {
                case ExoPlayer.STATE_IDLE:
                    notifyOnCompletion();
                    break;
                case ExoPlayer.STATE_PREPARING:
                    mIsPrepareing = true;
                    break;
                case ExoPlayer.STATE_BUFFERING:
                    notifyOnInfo(IMediaPlayer.MEDIA_INFO_BUFFERING_START, mInternalPlayer.getBufferedPercentage());
                    mIsBuffering = true;
                    break;
                case ExoPlayer.STATE_READY:
                    break;
                case ExoPlayer.STATE_ENDED:
                    notifyOnCompletion();
                    break;
                default:
                    break;
            }
        }

        public void onError(Exception e)
        {
            notifyOnError(IMediaPlayer.MEDIA_ERROR_UNKNOWN, IMediaPlayer.MEDIA_ERROR_UNKNOWN);
        }

        public void onVideoSizeChanged(int width, int height, int unappliedRotationDegrees,
                                float pixelWidthHeightRatio)
        {
            mVideoWidth = width;
            mVideoHeight = height;
            notifyOnVideoSizeChanged(width, height, 1, 1);
            if (unappliedRotationDegrees > 0)
                notifyOnInfo(IMediaPlayer.MEDIA_INFO_VIDEO_ROTATION_CHANGED, unappliedRotationDegrees);
        }
    }

    private DemoPlayerListener mDemoListener;
}
