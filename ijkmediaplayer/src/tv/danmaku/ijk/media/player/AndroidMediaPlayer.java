package tv.danmaku.ijk.media.player;

import java.io.IOException;

import android.content.Context;
import android.media.MediaPlayer;
import android.view.Surface;
import android.view.SurfaceHolder;

public class AndroidMediaPlayer extends AbstractMediaPlayer {
    private MediaPlayer mInternalMediaPlayer;

    public AndroidMediaPlayer() {
        mInternalMediaPlayer = new MediaPlayer();
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
    public void seekTo(int msec) throws IllegalStateException {
        mInternalMediaPlayer.seekTo(msec);
    }

    @Override
    public int getCurrentPosition() {
        return mInternalMediaPlayer.getCurrentPosition();
    }

    @Override
    public int getDuration() {
        return mInternalMediaPlayer.getDuration();
    }

    @Override
    public void release() {
        mInternalMediaPlayer.release();
    }

    @Override
    public void reset() {
        mInternalMediaPlayer.reset();
    }

    @Override
    public void setAudioStreamType(int streamtype) {
        mInternalMediaPlayer.setAudioStreamType(streamtype);
    }
}
