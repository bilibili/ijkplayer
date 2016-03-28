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

package tv.danmaku.ijk.media.sample.widget.media;

import android.annotation.TargetApi;
import android.app.Activity;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Point;
import android.graphics.SurfaceTexture;
import android.os.Build;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.util.AttributeSet;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Display;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityNodeInfo;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.lang.ref.WeakReference;
import java.nio.charset.Charset;
import java.util.Map;
import java.util.Scanner;
import java.util.concurrent.ConcurrentHashMap;

import tv.danmaku.ijk.media.player.IMediaPlayer;
import tv.danmaku.ijk.media.player.ISurfaceTextureHolder;

//
// AMLRenderView with special handling for AMLOGIC aspect ratio.
//
// It would be nice to just dervice from SurfaceRenderView but the measurement helper
// is private and we need access to it.
//
public class AMLRenderView extends SurfaceView implements IRenderView {
    private MeasureHelper mMeasureHelper;
    int m_videoWidth;
    int m_videoHeight;
    int m_aspectRatio;

    public AMLRenderView(Context context) {
        super(context);
        initView(context);
    }

    public AMLRenderView(Context context, AttributeSet attrs) {
        super(context, attrs);
        initView(context);
    }

    public AMLRenderView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        initView(context);
    }

    @TargetApi(Build.VERSION_CODES.LOLLIPOP)
    public AMLRenderView(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
        super(context, attrs, defStyleAttr, defStyleRes);
        initView(context);
    }

    private void initView(Context context) {
        mMeasureHelper = new MeasureHelper(this);
        mSurfaceCallback = new SurfaceCallback(this);
        getHolder().addCallback(mSurfaceCallback);
        //noinspection deprecation
        getHolder().setType(SurfaceHolder.SURFACE_TYPE_NORMAL);
    }

    @Override
    public View getView() {
        return this;
    }

    @Override
    public boolean shouldWaitForResize() {
        return true;
    }

    //--------------------
    // Layout & Measure
    //--------------------
    @Override
    public void setVideoSize(int videoWidth, int videoHeight) {
        if (videoWidth > 0 && videoHeight > 0) {
            mMeasureHelper.setVideoSize(videoWidth, videoHeight);
            getHolder().setFixedSize(videoWidth, videoHeight);
            requestLayout();
            m_videoWidth = videoWidth;
            m_videoHeight = videoHeight;
        }
    }

    @Override
    public void setVideoSampleAspectRatio(int videoSarNum, int videoSarDen) {
        if (videoSarNum > 0 && videoSarDen > 0) {
            mMeasureHelper.setVideoSampleAspectRatio(videoSarNum, videoSarDen);
            requestLayout();
        }
    }

    @Override
    public void setVideoRotation(int degree) {
        Log.e("", "SurfaceView doesn't support rotation (" + degree + ")!\n");
    }

    @Override
    public void setAspectRatio(int aspectRatio) {
        m_aspectRatio = aspectRatio;
        mMeasureHelper.setAspectRatio(aspectRatio);
        requestLayout();
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        //
        // The view width/height is the container size and in our case is normally
        // the framebuffer dimensions as we run full screen.
        //
        int viewWidth = View.getDefaultSize(m_videoWidth, widthMeasureSpec);
        int viewHeight = View.getDefaultSize(m_videoHeight, heightMeasureSpec);
        setMeasuredDimension(viewWidth, viewHeight);

        // Let ijk work out the measured dimensions according to the container size
        mMeasureHelper.doMeasure(widthMeasureSpec, heightMeasureSpec);

        int fitWidth = mMeasureHelper.getMeasuredWidth();
        int fitHeight = mMeasureHelper.getMeasuredHeight();


        //
        // The measure helper has fit the video resolution to the view but we
        // need to scale this up to the device resolution that amlogic is using.
        //
        // e.g. Android may be in a 1280x720 mode but amlogic is scanning that out
        // and presenting to the TV at 1920x1080.
        //
        // We need to supply the fit values in the scan out dimensions.
        //
        int[] deviceResolution = getDeviceResolution();
        if(deviceResolution == null)
            return;

        int deviceWidth  = deviceResolution[0];
        int deviceHeight = deviceResolution[1];

        Log.i("AMLRenderView", String.format("onMeasure: aspect:%d  device:%dx%d view:%dx%d  video:%dx%d  fit:%dx%d",
                m_aspectRatio, deviceWidth, deviceHeight, viewWidth, viewHeight, m_videoWidth, m_videoHeight, fitWidth, fitHeight));

        // scale factor from the view to the device coords
        float xScale = (float)deviceWidth / (float)viewWidth;
        float yScale = (float)deviceHeight / (float)viewHeight;

        // scale up the fit values
        float xx1 = fitWidth * xScale;
        float yy1 = fitHeight * yScale;

        // we need to create a centered rectangle for amlogic so work out the excess pixels and then
        // create a
        float xExcess = (xx1 - deviceWidth) * 0.5f;
        float yExcess = (yy1 - deviceHeight) * 0.5f;

        int x0 = (int)(-xExcess);
        int y0 = (int)(-yExcess);
        int x1 = (int)(xx1-xExcess);
        int y1 = (int)(yy1-yExcess);

        setScreenMode(1);   // full stretch. (e.g. use the axis values we supply)
        setAxis(x0, y0, x1, y1);
    }

    int [] getDeviceResolution()
    {
        try
        {
            Scanner s = new Scanner(new File("/sys/class/video/device_resolution")).useDelimiter("x|\\Z");

            int width = s.nextInt();
            int height = s.nextInt();

            return new int[] { width, height };
        }
        catch (IOException e) {
            e.printStackTrace();
        }

        return null;
    }

    void setScreenMode(int mode)
    {
        try {
            FileWriter w = new FileWriter("/sys/class/video/screen_mode");
            w.write(String.format("%d", mode));
            w.flush();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    void setAxis(int x0, int y0, int x1, int y1)
    {
        try {
            FileWriter w = new FileWriter("/sys/class/video/axis");
            w.write(String.format("%d %d %d %d", x0, y0, x1, y1));
            w.flush();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    //--------------------
    // SurfaceViewHolder
    //--------------------

    private static final class InternalSurfaceHolder implements ISurfaceHolder {
        private AMLRenderView mSurfaceView;
        private SurfaceHolder mSurfaceHolder;

        public InternalSurfaceHolder(@NonNull AMLRenderView surfaceView,
                                     @Nullable SurfaceHolder surfaceHolder) {
            mSurfaceView = surfaceView;
            mSurfaceHolder = surfaceHolder;
        }

        public void bindToMediaPlayer(IMediaPlayer mp) {
            if (mp != null) {
                if ((Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN) &&
                        (mp instanceof ISurfaceTextureHolder)) {
                    ISurfaceTextureHolder textureHolder = (ISurfaceTextureHolder) mp;
                    textureHolder.setSurfaceTexture(null);
                }
                mp.setDisplay(mSurfaceHolder);
            }
        }

        @NonNull
        @Override
        public IRenderView getRenderView() {
            return mSurfaceView;
        }

        @Nullable
        @Override
        public SurfaceHolder getSurfaceHolder() {
            return mSurfaceHolder;
        }

        @Nullable
        @Override
        public SurfaceTexture getSurfaceTexture() {
            return null;
        }

        @Nullable
        @Override
        public Surface openSurface() {
            if (mSurfaceHolder == null)
                return null;
            return mSurfaceHolder.getSurface();
        }
    }

    //-------------------------
    // SurfaceHolder.Callback
    //-------------------------

    @Override
    public void addRenderCallback(IRenderCallback callback) {
        mSurfaceCallback.addRenderCallback(callback);
    }

    @Override
    public void removeRenderCallback(IRenderCallback callback) {
        mSurfaceCallback.removeRenderCallback(callback);
    }

    private SurfaceCallback mSurfaceCallback;

    private static final class SurfaceCallback implements SurfaceHolder.Callback {
        private SurfaceHolder mSurfaceHolder;
        private boolean mIsFormatChanged;
        private int mFormat;
        private int mWidth;
        private int mHeight;

        private WeakReference<AMLRenderView> mWeakSurfaceView;
        private Map<IRenderCallback, Object> mRenderCallbackMap = new ConcurrentHashMap<IRenderCallback, Object>();

        public SurfaceCallback(@NonNull AMLRenderView surfaceView) {
            mWeakSurfaceView = new WeakReference<AMLRenderView>(surfaceView);
        }

        public void addRenderCallback(@NonNull IRenderCallback callback) {
            mRenderCallbackMap.put(callback, callback);

            ISurfaceHolder surfaceHolder = null;
            if (mSurfaceHolder != null) {
                if (surfaceHolder == null)
                    surfaceHolder = new InternalSurfaceHolder(mWeakSurfaceView.get(), mSurfaceHolder);
                callback.onSurfaceCreated(surfaceHolder, mWidth, mHeight);
            }

            if (mIsFormatChanged) {
                if (surfaceHolder == null)
                    surfaceHolder = new InternalSurfaceHolder(mWeakSurfaceView.get(), mSurfaceHolder);
                callback.onSurfaceChanged(surfaceHolder, mFormat, mWidth, mHeight);
            }
        }

        public void removeRenderCallback(@NonNull IRenderCallback callback) {
            mRenderCallbackMap.remove(callback);
        }

        @Override
        public void surfaceCreated(SurfaceHolder holder) {
            mSurfaceHolder = holder;
            mIsFormatChanged = false;
            mFormat = 0;
            mWidth = 0;
            mHeight = 0;

            ISurfaceHolder surfaceHolder = new InternalSurfaceHolder(mWeakSurfaceView.get(), mSurfaceHolder);
            for (IRenderCallback renderCallback : mRenderCallbackMap.keySet()) {
                renderCallback.onSurfaceCreated(surfaceHolder, 0, 0);
            }
        }

        @Override
        public void surfaceDestroyed(SurfaceHolder holder) {
            mSurfaceHolder = null;
            mIsFormatChanged = false;
            mFormat = 0;
            mWidth = 0;
            mHeight = 0;

            ISurfaceHolder surfaceHolder = new InternalSurfaceHolder(mWeakSurfaceView.get(), mSurfaceHolder);
            for (IRenderCallback renderCallback : mRenderCallbackMap.keySet()) {
                renderCallback.onSurfaceDestroyed(surfaceHolder);
            }
        }

        @Override
        public void surfaceChanged(SurfaceHolder holder, int format,
                                   int width, int height) {
            mSurfaceHolder = holder;
            mIsFormatChanged = true;
            mFormat = format;
            mWidth = width;
            mHeight = height;

            // mMeasureHelper.setVideoSize(width, height);

            ISurfaceHolder surfaceHolder = new InternalSurfaceHolder(mWeakSurfaceView.get(), mSurfaceHolder);
            for (IRenderCallback renderCallback : mRenderCallbackMap.keySet()) {
                renderCallback.onSurfaceChanged(surfaceHolder, format, width, height);
            }
        }
    }

    //--------------------
    // Accessibility
    //--------------------

    @Override
    public void onInitializeAccessibilityEvent(AccessibilityEvent event) {
        super.onInitializeAccessibilityEvent(event);
        event.setClassName(AMLRenderView.class.getName());
    }

    @TargetApi(Build.VERSION_CODES.ICE_CREAM_SANDWICH)
    @Override
    public void onInitializeAccessibilityNodeInfo(AccessibilityNodeInfo info) {
        super.onInitializeAccessibilityNodeInfo(info);
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.ICE_CREAM_SANDWICH) {
            info.setClassName(AMLRenderView.class.getName());
        }
    }
}
