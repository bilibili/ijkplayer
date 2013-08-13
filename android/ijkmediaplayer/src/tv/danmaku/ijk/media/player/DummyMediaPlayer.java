package tv.danmaku.ijk.media.player;

import java.io.IOException;

import android.content.Context;
import android.view.Surface;
import android.view.SurfaceHolder;

public class DummyMediaPlayer extends AbstractMediaPlayer {
    public int mVideoWidth;
    public int mVideoHeight;
    public long mCurrentPosition;
    public long mDuration;
    public boolean mIsPlaying;

    @Override
    public void setDisplay(SurfaceHolder sh) {
    }

    @Override
    public void setSurface(Surface surface) {
    }

    @Override
    public void setDataSource(String path) throws IOException,
            IllegalArgumentException, SecurityException, IllegalStateException {
    }

    @Override
    public void prepareAsync() throws IllegalStateException {
    }

    @Override
    public void start() throws IllegalStateException {
    }

    @Override
    public void stop() throws IllegalStateException {
    }

    @Override
    public void pause() throws IllegalStateException {
    }

    @Override
    public void setWakeMode(Context context, int mode) {
    }

    @Override
    public void setScreenOnWhilePlaying(boolean screenOn) {
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
        return mIsPlaying;
    }

    @Override
    public void seekTo(long msec) throws IllegalStateException {
    }

    @Override
    public long getCurrentPosition() {
        return mCurrentPosition;
    }

    @Override
    public long getDuration() {
        return mDuration;
    }

    @Override
    public void release() {
    }

    @Override
    public void reset() {
    }

    @Override
    public void setAudioStreamType(int streamtype) {
    }

    @Override
    public void setOnPreparedListener(OnPreparedListener listener) {
    }

    @Override
    public void setOnCompletionListener(OnCompletionListener listener) {
    }

    @Override
    public void setOnBufferingUpdateListener(OnBufferingUpdateListener listener) {
    }

    @Override
    public void setOnSeekCompleteListener(OnSeekCompleteListener listener) {
    }

    @Override
    public void setOnVideoSizeChangedListener(
            OnVideoSizeChangedListener listener) {
    }

    @Override
    public void setOnErrorListener(OnErrorListener listener) {
    }

    @Override
    public void setOnInfoListener(OnInfoListener listener) {
    }
}
