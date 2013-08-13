package tv.danmaku.ijk.media.player.list;

import java.io.IOException;

import tv.danmaku.ijk.media.player.AbstractMediaPlayer;
import tv.danmaku.ijk.media.player.DummyMediaPlayer;
import android.content.Context;
import android.text.TextUtils;
import android.view.Surface;
import android.view.SurfaceHolder;

public final class SimpleMediaSegmentPlayer extends AbstractMediaPlayer {
    private int mOrder;

    private MediaSegment mMediaSegment = new MediaSegment();
    private AbstractMediaPlayer mMediaPlayer;
    private DummyMediaPlayer mDummyPlayer = new DummyMediaPlayer();

    private boolean mIsReleased;
    private Long mSeekOnPrepared;

    public SimpleMediaSegmentPlayer(int order, AbstractMediaPlayer mediaPlayer) {
        mOrder = order;
        mMediaPlayer = mediaPlayer;
    }

    public int getOrder() {
        return mOrder;
    }

    public void setSeekOnPrepared(long seekOnPrepared) {
        mSeekOnPrepared = seekOnPrepared;
    }

    public boolean hasDataSource() {
        if (mMediaSegment == null || TextUtils.isEmpty(mMediaSegment.mUrl))
            return false;

        return true;
    }

    public boolean isSameMediaItem(MediaSegment mediaItem) {
        if (!hasDataSource())
            return false;

        if (mMediaSegment.mOrder != mediaItem.mOrder)
            return false;

        return true;
    }

    public void setMediaSegment(MediaSegment mediaSegment) {
        mOrder = mediaSegment.mOrder;
        mMediaSegment = mediaSegment;
        mDummyPlayer.mDuration = mediaSegment.mDuration;
    }

    public void setDisplay(SurfaceHolder sh) {
        mMediaPlayer.setDisplay(sh);
    }

    public void setSurface(Surface surface) {
        mMediaPlayer.setSurface(surface);
    }

    public void seekToAbsoluteTime(long msec) throws IllegalStateException {
        long relativeTime = msec - mMediaSegment.mStartTime;
        relativeTime = Math.max(0, relativeTime);

        mMediaPlayer.seekTo(relativeTime);
    }

    public long getAbsolutePosition() {
        long absolutePosition = mMediaSegment.mStartTime
                + mMediaPlayer.getCurrentPosition();
        return absolutePosition;
    }

    public boolean isReleased() {
        return mIsReleased;
    }

    public void releaseToDummyPlayer(boolean pretendPlaying, long fakePosition) {
        mDummyPlayer.mCurrentPosition = fakePosition;
        mDummyPlayer.mIsPlaying = pretendPlaying;

        AbstractMediaPlayer prevMediaPlayer = mMediaPlayer;
        mMediaPlayer = mDummyPlayer;
        prevMediaPlayer.release();
    }

    public void releaseToDummyPlayer(boolean pretendPlaying) {
        releaseToDummyPlayer(pretendPlaying, getDuration());
    }

    public void startOnPrepared() {
        retrieveMedioInfo();
        mDummyPlayer.mIsPlaying = true;

        start();
        if (mSeekOnPrepared != null) {
            mDummyPlayer.mCurrentPosition = mSeekOnPrepared;
            seekTo(mSeekOnPrepared);
            mSeekOnPrepared = null;
        } else {
            mDummyPlayer.mCurrentPosition = 0;
        }
    }

    private void retrieveMedioInfo() {
        int videoWidth = mMediaPlayer.getVideoWidth();
        if (videoWidth > 0)
            mDummyPlayer.mVideoWidth = videoWidth;

        int videoHeight = mMediaPlayer.getVideoHeight();
        if (videoHeight > 0)
            mDummyPlayer.mVideoHeight = videoHeight;

        long duration = mMediaPlayer.getDuration();
        if (duration > 0)
            mDummyPlayer.mDuration = duration;
    }

    @Override
    public void setDataSource(String path) throws IOException,
            IllegalArgumentException, SecurityException, IllegalStateException {
        mMediaSegment = new MediaSegment(path);
    }

    @Override
    public void prepareAsync() throws IllegalStateException {
        try {
            if (hasDataSource()) {
                mMediaPlayer.setDataSource(mMediaSegment.mUrl);
                mMediaPlayer.prepareAsync();
            } else
                throw new IllegalStateException("null data source");
        } catch (IllegalArgumentException e) {
            throw new IllegalStateException(e);
        } catch (SecurityException e) {
            throw new IllegalStateException(e);
        } catch (IOException e) {
            throw new IllegalStateException(e);
        }
    }

    @Override
    public void start() throws IllegalStateException {
        mMediaPlayer.start();
    }

    @Override
    public void stop() throws IllegalStateException {
        mMediaPlayer.stop();
    }

    @Override
    public void pause() throws IllegalStateException {
        mMediaPlayer.pause();
    }

    @Override
    public void setWakeMode(Context context, int mode) {
        mMediaPlayer.setWakeMode(context, mode);
    }

    @Override
    public void setScreenOnWhilePlaying(boolean screenOn) {
        mMediaPlayer.setScreenOnWhilePlaying(screenOn);
    }

    @Override
    public int getVideoWidth() {
        int videoWidth = mMediaPlayer.getVideoWidth();
        if (videoWidth > 0)
            mDummyPlayer.mVideoWidth = videoWidth;
        return videoWidth;
    }

    @Override
    public int getVideoHeight() {
        int videoHeight = mMediaPlayer.getVideoHeight();
        if (videoHeight > 0)
            mDummyPlayer.mVideoHeight = videoHeight;
        return mMediaPlayer.getVideoHeight();
    }

    @Override
    public boolean isPlaying() {
        return mMediaPlayer.isPlaying();
    }

    @Override
    public void seekTo(long msec) throws IllegalStateException {
        mMediaPlayer.seekTo(msec);
    }

    @Override
    public long getCurrentPosition() {
        return mMediaPlayer.getCurrentPosition();
    }

    @Override
    public long getDuration() {
        if (mMediaSegment.mDuration > 0) {
            return mMediaSegment.mDuration;
        }

        long duration = mMediaPlayer.getDuration();
        if (duration > 0)
            mDummyPlayer.mDuration = duration;

        return duration;
    }

    @Override
    public void release() {
        mIsReleased = true;
        mMediaPlayer.release();
    }

    @Override
    public void reset() {
        mOrder = -1;
        mMediaSegment = null;
        mMediaPlayer.reset();
    }

    @Override
    public void setAudioStreamType(int streamtype) {
        mMediaPlayer.setAudioStreamType(streamtype);
    }

    @Override
    public void setOnPreparedListener(OnPreparedListener listener) {
        mMediaPlayer.setOnPreparedListener(listener);
    }

    @Override
    public void setOnCompletionListener(OnCompletionListener listener) {
        mMediaPlayer.setOnCompletionListener(listener);
    }

    @Override
    public void setOnBufferingUpdateListener(OnBufferingUpdateListener listener) {
        mMediaPlayer.setOnBufferingUpdateListener(listener);
    }

    @Override
    public void setOnSeekCompleteListener(OnSeekCompleteListener listener) {
        mMediaPlayer.setOnSeekCompleteListener(listener);
    }

    @Override
    public void setOnVideoSizeChangedListener(
            OnVideoSizeChangedListener listener) {
        mMediaPlayer.setOnVideoSizeChangedListener(listener);
    }

    @Override
    public void setOnErrorListener(OnErrorListener listener) {
        mMediaPlayer.setOnErrorListener(listener);
    }

    @Override
    public void setOnInfoListener(OnInfoListener listener) {
        mMediaPlayer.setOnInfoListener(listener);
    }
}
