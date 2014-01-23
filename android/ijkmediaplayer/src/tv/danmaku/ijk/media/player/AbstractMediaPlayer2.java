package tv.danmaku.ijk.media.player;

import android.annotation.TargetApi;
import android.content.Context;
import android.os.Build;
import android.view.Surface;

// rarely used api
public abstract class AbstractMediaPlayer2 extends AbstractMediaPlayer {
    @TargetApi(Build.VERSION_CODES.ICE_CREAM_SANDWICH)
    public abstract void setSurface(Surface surface);

    public abstract void setWakeMode(Context context, int mode);

    public abstract void setAudioStreamType(int streamtype);
}
