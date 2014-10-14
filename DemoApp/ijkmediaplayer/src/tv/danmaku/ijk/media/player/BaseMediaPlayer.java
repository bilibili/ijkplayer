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
