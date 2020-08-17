package tv.danmaku.ijk.media.player.misc;

import android.graphics.SurfaceTexture;
import android.view.Surface;

@SimpleCClassName
public class MediaCodecSurface {

    public Surface getSurface();

    public SurfaceTexture getSurfaceTexture();

    public void updateTexImage(float[] mtx);

    public void attachToGLContext(int texName, int width, int height);

    public void detachFromGLContext();

    public void release();
}
