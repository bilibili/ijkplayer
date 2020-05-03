package tv.danmaku.ijk.media.player.misc;

import android.graphics.SurfaceTexture;
import android.util.Log;
import android.view.Surface;

import tv.danmaku.ijk.media.player.annotations.AccessedByNative;
import tv.danmaku.ijk.media.player.annotations.CalledByNative;
import tv.danmaku.ijk.media.player.pragma.DebugLog;

public class MediaCodecSurface {

    private Surface mSurface;
    private SurfaceTexture mSurfaceTexture;
    private boolean mReleased;
    private boolean mAttached;

    public MediaCodecSurface() {
        mSurfaceTexture = new SurfaceTexture(0);
        mSurface = new Surface(mSurfaceTexture);
        mReleased = false;
        mAttached = false;
        /*
        mSurfaceTexture.setOnFrameAvailableListener(new SurfaceTexture.OnFrameAvailableListener() {
            @Override
            public void onFrameAvailable(SurfaceTexture surfaceTexture) {
                Log.d("Surface", "onFrameAvailable");
            }
        });
         */

        mSurfaceTexture.detachFromGLContext();
    }

    @CalledByNative
    public Surface getSurface() {
        if (mReleased)
            return null;
        return mSurface;
    }

    @CalledByNative
    public SurfaceTexture getSurfaceTexture() {
        if (mReleased)
            return null;
        return mSurfaceTexture;
    }

    @CalledByNative
    public void updateTexImage(float[] mtx) {
        if (mReleased || !mAttached)
            return;
        mSurfaceTexture.updateTexImage();
        mSurfaceTexture.getTransformMatrix(mtx);
    }

    @CalledByNative
    public void attachToGLContext(int texName, int width, int height) {
        if (mReleased || mAttached)
            return;
        // mSurfaceTexture.detachFromGLContext();
        mAttached = true;
        mSurfaceTexture.attachToGLContext(texName);
        // mSurfaceTexture.setDefaultBufferSize(width, height);
    }


    @CalledByNative
    public void detachFromGLContext() {
        if (mAttached) {
            mSurfaceTexture.detachFromGLContext();
            mAttached = false;
        }
    }

    @CalledByNative
    public void release() {
        DebugLog.d("MediaCodecSurface", "release()," + mReleased);
        mReleased = true;
        //mSurfaceTexture.detachFromGLContext();
        if (mSurfaceTexture != null) {
            mSurfaceTexture.release();
            mSurfaceTexture = null;
        }
        if (mSurface != null) {
            mSurface.release();
            mSurface = null;
        }
    }
}
