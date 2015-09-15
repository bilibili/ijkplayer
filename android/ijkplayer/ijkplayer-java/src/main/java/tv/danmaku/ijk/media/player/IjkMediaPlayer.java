/*
 * Copyright (C) 2006 The Android Open Source Project
 * Copyright (C) 2013 Zhang Rui <bbcallen@gmail.com>
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

import android.annotation.SuppressLint;
import android.annotation.TargetApi;
import android.content.ContentResolver;
import android.content.Context;
import android.content.res.AssetFileDescriptor;
import android.graphics.SurfaceTexture;
import android.media.MediaCodecInfo;
import android.media.MediaCodecList;
import android.media.RingtoneManager;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.ParcelFileDescriptor;
import android.os.PowerManager;
import android.provider.Settings;
import android.text.TextUtils;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;

import java.io.File;
import java.io.FileDescriptor;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.lang.ref.WeakReference;
import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.Locale;
import java.util.Map;

import tv.danmaku.ijk.media.player.annotations.AccessedByNative;
import tv.danmaku.ijk.media.player.annotations.CalledByNative;
import tv.danmaku.ijk.media.player.pragma.DebugLog;

/**
 * @author bbcallen
 * 
 *         Java wrapper of ffplay.
 */
public final class IjkMediaPlayer extends AbstractMediaPlayer {
    private final static String TAG = IjkMediaPlayer.class.getName();

    private static final int MEDIA_NOP = 0; // interface test message
    private static final int MEDIA_PREPARED = 1;
    private static final int MEDIA_PLAYBACK_COMPLETE = 2;
    private static final int MEDIA_BUFFERING_UPDATE = 3;
    private static final int MEDIA_SEEK_COMPLETE = 4;
    private static final int MEDIA_SET_VIDEO_SIZE = 5;
    private static final int MEDIA_TIMED_TEXT = 99;
    private static final int MEDIA_ERROR = 100;
    private static final int MEDIA_INFO = 200;

    protected static final int MEDIA_SET_VIDEO_SAR = 10001;

    //----------------------------------------
    // options
    public static final int IJK_LOG_UNKNOWN = 0;
    public static final int IJK_LOG_DEFAULT = 1;

    public static final int IJK_LOG_VERBOSE = 2;
    public static final int IJK_LOG_DEBUG = 3;
    public static final int IJK_LOG_INFO = 4;
    public static final int IJK_LOG_WARN = 5;
    public static final int IJK_LOG_ERROR = 6;
    public static final int IJK_LOG_FATAL = 7;
    public static final int IJK_LOG_SILENT = 8;

    public static final int OPT_CATEGORY_FORMAT     = 1;
    public static final int OPT_CATEGORY_CODEC      = 2;
    public static final int OPT_CATEGORY_SWS        = 3;
    public static final int OPT_CATEGORY_PLAYER     = 4;

    public static final int SDL_FCC_YV12 = 0x32315659; // YV12
    public static final int SDL_FCC_RV16 = 0x36315652; // RGB565
    public static final int SDL_FCC_RV32 = 0x32335652; // RGBX8888
    //----------------------------------------

    @AccessedByNative
    private long mNativeMediaPlayer;

    @AccessedByNative
    private int mNativeSurfaceTexture;

    @AccessedByNative
    private int mListenerContext;

    private SurfaceHolder mSurfaceHolder;
    private EventHandler mEventHandler;
    private PowerManager.WakeLock mWakeLock = null;
    private boolean mScreenOnWhilePlaying;
    private boolean mStayAwake;

    private int mVideoWidth;
    private int mVideoHeight;
    private int mVideoSarNum;
    private int mVideoSarDen;

    private String mDataSource;

    /**
     * Default library loader
     * Load them by yourself, if your libraries are not installed at default place.
     */
    private static IjkLibLoader sLocalLibLoader = new IjkLibLoader() {
        @Override
        public void loadLibrary(String libName) throws UnsatisfiedLinkError, SecurityException {
            System.loadLibrary(libName);
        }
    };

    private static volatile boolean mIsLibLoaded = false;
    public static void loadLibrariesOnce(IjkLibLoader libLoader) {
        synchronized (IjkMediaPlayer.class) {
            if (!mIsLibLoaded) {
                if (libLoader == null)
                    libLoader = sLocalLibLoader;

                libLoader.loadLibrary("ijkffmpeg");
                libLoader.loadLibrary("ijksdl");
                libLoader.loadLibrary("ijkplayer");
                mIsLibLoaded = true;
            }
        }
    }

    private static volatile boolean mIsNativeInitialized = false;
    private static void initNativeOnce() {
        synchronized (IjkMediaPlayer.class) {
            if (!mIsNativeInitialized) {
                native_init();
                mIsNativeInitialized = true;
            }
        }
    }

    /**
     * Default constructor. Consider using one of the create() methods for
     * synchronously instantiating a IjkMediaPlayer from a Uri or resource.
     * <p>
     * When done with the IjkMediaPlayer, you should call {@link #release()}, to
     * free the resources. If not released, too many IjkMediaPlayer instances
     * may result in an exception.
     * </p>
     */
    public IjkMediaPlayer() {
        this(sLocalLibLoader);
    }

    /**
     * do not loadLibaray
     * @param libLoader
     *              custom library loader, can be null.
     */
    public IjkMediaPlayer(IjkLibLoader libLoader) {
        initPlayer(libLoader);
    }

    private void initPlayer(IjkLibLoader libLoader) {
        loadLibrariesOnce(libLoader);
        initNativeOnce();

        Looper looper;
        if ((looper = Looper.myLooper()) != null) {
            mEventHandler = new EventHandler(this, looper);
        } else if ((looper = Looper.getMainLooper()) != null) {
            mEventHandler = new EventHandler(this, looper);
        } else {
            mEventHandler = null;
        }

        /*
         * Native setup requires a weak reference to our object. It's easier to
         * create it here than in C++.
         */
        native_setup(new WeakReference<IjkMediaPlayer>(this));
    }

    /*
     * Update the IjkMediaPlayer SurfaceTexture. Call after setting a new
     * display surface.
     */
    private native void _setVideoSurface(Surface surface);

    /**
     * Sets the {@link SurfaceHolder} to use for displaying the video portion of
     * the media.
     * 
     * Either a surface holder or surface must be set if a display or video sink
     * is needed. Not calling this method or {@link #setSurface(Surface)} when
     * playing back a video will result in only the audio track being played. A
     * null surface holder or surface will result in only the audio track being
     * played.
     * 
     * @param sh
     *            the SurfaceHolder to use for video display
     */
    @Override
    public void setDisplay(SurfaceHolder sh) {
        mSurfaceHolder = sh;
        Surface surface;
        if (sh != null) {
            surface = sh.getSurface();
        } else {
            surface = null;
        }
        _setVideoSurface(surface);
        updateSurfaceScreenOn();
    }

    /**
     * Sets the {@link Surface} to be used as the sink for the video portion of
     * the media. This is similar to {@link #setDisplay(SurfaceHolder)}, but
     * does not support {@link #setScreenOnWhilePlaying(boolean)}. Setting a
     * Surface will un-set any Surface or SurfaceHolder that was previously set.
     * A null surface will result in only the audio track being played.
     * 
     * If the Surface sends frames to a {@link SurfaceTexture}, the timestamps
     * returned from {@link SurfaceTexture#getTimestamp()} will have an
     * unspecified zero point. These timestamps cannot be directly compared
     * between different media sources, different instances of the same media
     * source, or multiple runs of the same program. The timestamp is normally
     * monotonically increasing and is unaffected by time-of-day adjustments,
     * but it is reset when the position is set.
     * 
     * @param surface
     *            The {@link Surface} to be used for the video portion of the
     *            media.
     */
    @Override
    public void setSurface(Surface surface) {
        if (mScreenOnWhilePlaying && surface != null) {
            DebugLog.w(TAG,
                    "setScreenOnWhilePlaying(true) is ineffective for Surface");
        }
        mSurfaceHolder = null;
        _setVideoSurface(surface);
        updateSurfaceScreenOn();
    }

    /**
     * Sets the data source as a content Uri.
     *
     * @param context the Context to use when resolving the Uri
     * @param uri the Content URI of the data you want to play
     * @throws IllegalStateException if it is called in an invalid state
     */
    @Override
    public void setDataSource(Context context, Uri uri)
            throws IOException, IllegalArgumentException, SecurityException, IllegalStateException {
        setDataSource(context, uri, null);
    }

    /**
     * Sets the data source as a content Uri.
     *
     * @param context the Context to use when resolving the Uri
     * @param uri the Content URI of the data you want to play
     * @param headers the headers to be sent together with the request for the data
     *                Note that the cross domain redirection is allowed by default, but that can be
     *                changed with key/value pairs through the headers parameter with
     *                "android-allow-cross-domain-redirect" as the key and "0" or "1" as the value
     *                to disallow or allow cross domain redirection.
     * @throws IllegalStateException if it is called in an invalid state
     */
    @TargetApi(Build.VERSION_CODES.ICE_CREAM_SANDWICH)
    @Override
    public void setDataSource(Context context, Uri uri, Map<String, String> headers)
            throws IOException, IllegalArgumentException, SecurityException, IllegalStateException {
        final String scheme = uri.getScheme();
        if (ContentResolver.SCHEME_FILE.equals(scheme)) {
            setDataSource(uri.getPath());
            return;
        } else if (ContentResolver.SCHEME_CONTENT.equals(scheme)
                && Settings.AUTHORITY.equals(uri.getAuthority())) {
            // Redirect ringtones to go directly to underlying provider
            uri = RingtoneManager.getActualDefaultRingtoneUri(context,
                    RingtoneManager.getDefaultType(uri));
            if (uri == null) {
                throw new FileNotFoundException("Failed to resolve default ringtone");
            }
        }

        AssetFileDescriptor fd = null;
        try {
            ContentResolver resolver = context.getContentResolver();
            fd = resolver.openAssetFileDescriptor(uri, "r");
            if (fd == null) {
                return;
            }
            // Note: using getDeclaredLength so that our behavior is the same
            // as previous versions when the content provider is returning
            // a full file.
            if (fd.getDeclaredLength() < 0) {
                setDataSource(fd.getFileDescriptor());
            } else {
                setDataSource(fd.getFileDescriptor(), fd.getStartOffset(), fd.getDeclaredLength());
            }
            return;
        } catch (SecurityException ex) {
        } catch (IOException ex) {
        } finally {
            if (fd != null) {
                fd.close();
            }
        }

        Log.d(TAG, "Couldn't open file on client side, trying server side");

        setDataSource(uri.toString(), headers);
    }

    /**
     * Sets the data source (file-path or http/rtsp URL) to use.
     * 
     * @param path
     *            the path of the file, or the http/rtsp URL of the stream you
     *            want to play
     * @throws IllegalStateException
     *             if it is called in an invalid state
     * 
     *             <p>
     *             When <code>path</code> refers to a local file, the file may
     *             actually be opened by a process other than the calling
     *             application. This implies that the pathname should be an
     *             absolute path (as any other process runs with unspecified
     *             current working directory), and that the pathname should
     *             reference a world-readable file.
     */
    @Override
    public void setDataSource(String path)
            throws IOException, IllegalArgumentException, SecurityException, IllegalStateException {
        mDataSource = path;
        _setDataSource(path, null, null);
    }

    /**
     * Sets the data source (file-path or http/rtsp URL) to use.
     *
     * @param path the path of the file, or the http/rtsp URL of the stream you want to play
     * @param headers the headers associated with the http request for the stream you want to play
     * @throws IllegalStateException if it is called in an invalid state
     */
    public void setDataSource(String path, Map<String, String> headers)
            throws IOException, IllegalArgumentException, SecurityException, IllegalStateException
    {
        if (headers != null && !headers.isEmpty()) {
            StringBuilder sb = new StringBuilder();
            for(Map.Entry<String, String> entry: headers.entrySet()) {
                sb.append(entry.getKey());
                sb.append(":");
                String value = entry.getValue();
                if (!TextUtils.isEmpty(value))
                    sb.append(entry.getValue());
                sb.append("\r\n");
            }
        }
        setDataSource(path);
    }

    /**
     * Sets the data source (FileDescriptor) to use. It is the caller's responsibility
     * to close the file descriptor. It is safe to do so as soon as this call returns.
     *
     * @param fd the FileDescriptor for the file you want to play
     * @throws IllegalStateException if it is called in an invalid state
     */
    @TargetApi(Build.VERSION_CODES.HONEYCOMB_MR2)
    @Override
    public void setDataSource(FileDescriptor fd)
            throws IOException, IllegalArgumentException, IllegalStateException {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.HONEYCOMB_MR1) {
            int native_fd = -1;
            try {
                Field f = fd.getClass().getDeclaredField("descriptor"); //NoSuchFieldException
                f.setAccessible(true);
                native_fd = f.getInt(fd); //IllegalAccessException
            } catch (NoSuchFieldException e) {
                throw new RuntimeException(e);
            } catch (IllegalAccessException e) {
                throw new RuntimeException(e);
            }
            _setDataSourceFd(native_fd);
        } else {
            ParcelFileDescriptor pfd = ParcelFileDescriptor.dup(fd);
            try {
                _setDataSourceFd(pfd.getFd());
            } finally {
                pfd.close();
            }
        }
    }

    /**
     * Sets the data source (FileDescriptor) to use.  The FileDescriptor must be
     * seekable (N.B. a LocalSocket is not seekable). It is the caller's responsibility
     * to close the file descriptor. It is safe to do so as soon as this call returns.
     *
     * @param fd the FileDescriptor for the file you want to play
     * @param offset the offset into the file where the data to be played starts, in bytes
     * @param length the length in bytes of the data to be played
     * @throws IllegalStateException if it is called in an invalid state
     */
    private void setDataSource(FileDescriptor fd, long offset, long length)
            throws IOException, IllegalArgumentException, IllegalStateException {
        // FIXME: handle offset, length
        setDataSource(fd);
    }

    private native void _setDataSource(String path, String[] keys, String[] values)
            throws IOException, IllegalArgumentException, SecurityException, IllegalStateException;

    private native void _setDataSourceFd(int fd)
            throws IOException, IllegalArgumentException, SecurityException, IllegalStateException;

    @Override
    public String getDataSource() {
        return mDataSource;
    }

    @Override
    public void prepareAsync() throws IllegalStateException {
        _prepareAsync();
    }

    public native void _prepareAsync() throws IllegalStateException;

    @Override
    public void start() throws IllegalStateException {
        stayAwake(true);
        _start();
    }

    private native void _start() throws IllegalStateException;

    @Override
    public void stop() throws IllegalStateException {
        stayAwake(false);
        _stop();
    }

    private native void _stop() throws IllegalStateException;

    @Override
    public void pause() throws IllegalStateException {
        stayAwake(false);
        _pause();
    }

    private native void _pause() throws IllegalStateException;

    @SuppressLint("Wakelock")
    @Override
    public void setWakeMode(Context context, int mode) {
        boolean washeld = false;
        if (mWakeLock != null) {
            if (mWakeLock.isHeld()) {
                washeld = true;
                mWakeLock.release();
            }
            mWakeLock = null;
        }

        PowerManager pm = (PowerManager) context
                .getSystemService(Context.POWER_SERVICE);
        mWakeLock = pm.newWakeLock(mode | PowerManager.ON_AFTER_RELEASE,
                IjkMediaPlayer.class.getName());
        mWakeLock.setReferenceCounted(false);
        if (washeld) {
            mWakeLock.acquire();
        }
    }

    @Override
    public void setScreenOnWhilePlaying(boolean screenOn) {
        if (mScreenOnWhilePlaying != screenOn) {
            if (screenOn && mSurfaceHolder == null) {
                DebugLog.w(TAG,
                        "setScreenOnWhilePlaying(true) is ineffective without a SurfaceHolder");
            }
            mScreenOnWhilePlaying = screenOn;
            updateSurfaceScreenOn();
        }
    }

    @SuppressLint("Wakelock")
    private void stayAwake(boolean awake) {
        if (mWakeLock != null) {
            if (awake && !mWakeLock.isHeld()) {
                mWakeLock.acquire();
            } else if (!awake && mWakeLock.isHeld()) {
                mWakeLock.release();
            }
        }
        mStayAwake = awake;
        updateSurfaceScreenOn();
    }

    private void updateSurfaceScreenOn() {
        if (mSurfaceHolder != null) {
            mSurfaceHolder.setKeepScreenOn(mScreenOnWhilePlaying && mStayAwake);
        }
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
    public int getVideoSarNum() {
        return mVideoSarNum;
    }

    @Override
    public int getVideoSarDen() {
        return mVideoSarDen;
    }

    @Override
    public native boolean isPlaying();

    @Override
    public native void seekTo(long msec) throws IllegalStateException;

    @Override
    public native long getCurrentPosition();

    @Override
    public native long getDuration();

    /**
     * Releases resources associated with this IjkMediaPlayer object. It is
     * considered good practice to call this method when you're done using the
     * IjkMediaPlayer. In particular, whenever an Activity of an application is
     * paused (its onPause() method is called), or stopped (its onStop() method
     * is called), this method should be invoked to release the IjkMediaPlayer
     * object, unless the application has a special need to keep the object
     * around. In addition to unnecessary resources (such as memory and
     * instances of codecs) being held, failure to call this method immediately
     * if a IjkMediaPlayer object is no longer needed may also lead to
     * continuous battery consumption for mobile devices, and playback failure
     * for other applications if no multiple instances of the same codec are
     * supported on a device. Even if multiple instances of the same codec are
     * supported, some performance degradation may be expected when unnecessary
     * multiple instances are used at the same time.
     */
    @Override
    public void release() {
        stayAwake(false);
        updateSurfaceScreenOn();
        resetListeners();
        _release();
    }

    private native void _release();

    @Override
    public void reset() {
        stayAwake(false);
        _reset();
        // make sure none of the listeners get called anymore
        mEventHandler.removeCallbacksAndMessages(null);

        mVideoWidth = 0;
        mVideoHeight = 0;
    }

    private native void _reset();

    public native void setVolume(float leftVolume, float rightVolume);

    @Override
    public MediaInfo getMediaInfo() {
        MediaInfo mediaInfo = new MediaInfo();
        mediaInfo.mMediaPlayerName = "ijkplayer";

        String videoCodecInfo = _getVideoCodecInfo();
        if (!TextUtils.isEmpty(videoCodecInfo)) {
            String nodes[] = videoCodecInfo.split(",");
            if (nodes.length >= 2) {
                mediaInfo.mVideoDecoder = nodes[0];
                mediaInfo.mVideoDecoderImpl = nodes[1];
            } else if (nodes.length >= 1) {
                mediaInfo.mVideoDecoder = nodes[0];
                mediaInfo.mVideoDecoderImpl = "";
            }
        }

        String audioCodecInfo = _getAudioCodecInfo();
        if (!TextUtils.isEmpty(audioCodecInfo)) {
            String nodes[] = audioCodecInfo.split(",");
            if (nodes.length >= 2) {
                mediaInfo.mAudioDecoder = nodes[0];
                mediaInfo.mAudioDecoderImpl = nodes[1];
            } else if (nodes.length >= 1) {
                mediaInfo.mAudioDecoder = nodes[0];
                mediaInfo.mAudioDecoderImpl = "";
            }
        }

        try {
            mediaInfo.mMeta = IjkMediaMeta.parse(_getMediaMeta());
        } catch (Throwable e) {
            e.printStackTrace();
        }
        return mediaInfo;
    }

    @Override
    public void setLogEnabled(boolean enable) {
        // do nothing
    }

    @Override
    public boolean isPlayable() {
        return true;
    }

    private native String _getVideoCodecInfo();
    private native String _getAudioCodecInfo();

    public void setOption(int category, String name, String value)
    {
        _setOption(category, name, value);
    }

    public void setOption(int category, String name, long value)
    {
        _setOption(category, name, value);
    }

    private native void _setOption(int category, String name, String value);
    private native void _setOption(int category, String name, long value);

    public Bundle getMediaMeta() {
        return _getMediaMeta();
    }
    private native Bundle _getMediaMeta();

    public static String getColorFormatName(int mediaCodecColorFormat) {
        return _getColorFormatName(mediaCodecColorFormat);
    }

    private static native String _getColorFormatName(int mediaCodecColorFormat);

    @Override
    public void setAudioStreamType(int streamtype) {
        // do nothing
    }

    @Override
    public void setKeepInBackground(boolean keepInBackground) {
        // do nothing
    }

    private static native void native_init();

    private native void native_setup(Object IjkMediaPlayer_this);

    private native void native_finalize();

    private native void native_message_loop(Object IjkMediaPlayer_this);

    protected void finalize() {
        native_finalize();
    }

    private static class EventHandler extends Handler {
        private WeakReference<IjkMediaPlayer> mWeakPlayer;

        public EventHandler(IjkMediaPlayer mp, Looper looper) {
            super(looper);
            mWeakPlayer = new WeakReference<IjkMediaPlayer>(mp);
        }

        @Override
        public void handleMessage(Message msg) {
            IjkMediaPlayer player = mWeakPlayer.get();
            if (player == null || player.mNativeMediaPlayer == 0) {
                DebugLog.w(TAG,
                        "IjkMediaPlayer went away with unhandled events");
                return;
            }

            switch (msg.what) {
            case MEDIA_PREPARED:
                player.notifyOnPrepared();
                return;

            case MEDIA_PLAYBACK_COMPLETE:
                player.notifyOnCompletion();
                player.stayAwake(false);
                return;

            case MEDIA_BUFFERING_UPDATE:
                long bufferPosition = msg.arg1;
                if (bufferPosition < 0) {
                    bufferPosition = 0;
                }

                long percent = 0;
                long duration = player.getDuration();
                if (duration > 0) {
                    percent = bufferPosition * 100 / duration;
                }
                if (percent >= 100) {
                    percent = 100;
                }

                // DebugLog.efmt(TAG, "Buffer (%d%%) %d/%d",  percent, bufferPosition, duration);
                player.notifyOnBufferingUpdate((int)percent);
                return;

            case MEDIA_SEEK_COMPLETE:
                player.notifyOnSeekComplete();
                return;

            case MEDIA_SET_VIDEO_SIZE:
                player.mVideoWidth = msg.arg1;
                player.mVideoHeight = msg.arg2;
                player.notifyOnVideoSizeChanged(player.mVideoWidth, player.mVideoHeight,
                        player.mVideoSarNum, player.mVideoSarDen);
                return;

            case MEDIA_ERROR:
                DebugLog.e(TAG, "Error (" + msg.arg1 + "," + msg.arg2 + ")");
                if (!player.notifyOnError(msg.arg1, msg.arg2)) {
                    player.notifyOnCompletion();
                }
                player.stayAwake(false);
                return;

            case MEDIA_INFO:
                switch (msg.arg1) {
                    case MEDIA_INFO_VIDEO_RENDERING_START:
                        DebugLog.i(TAG, "Info: MEDIA_INFO_VIDEO_RENDERING_START\n");
                        break;
                }
                player.notifyOnInfo(msg.arg1, msg.arg2);
                // No real default action so far.
                return;
            case MEDIA_TIMED_TEXT:
                // do nothing
                break;

            case MEDIA_NOP: // interface test message - ignore
                break;

            case MEDIA_SET_VIDEO_SAR:
                player.mVideoSarNum = msg.arg1;
                player.mVideoSarDen = msg.arg2;
                player.notifyOnVideoSizeChanged(player.mVideoWidth, player.mVideoHeight,
                        player.mVideoSarNum, player.mVideoSarDen);
                break;

            default:
                DebugLog.e(TAG, "Unknown message type " + msg.what);
                return;
            }
        }
    }

    /*
     * Called from native code when an interesting event happens. This method
     * just uses the EventHandler system to post the event back to the main app
     * thread. We use a weak reference to the original IjkMediaPlayer object so
     * that the native code is safe from the object disappearing from underneath
     * it. (This is the cookie passed to native_setup().)
     */
    @CalledByNative
    private static void postEventFromNative(Object weakThiz, int what,
            int arg1, int arg2, Object obj) {
        if (weakThiz == null)
            return;

        @SuppressWarnings("rawtypes")
        IjkMediaPlayer mp = (IjkMediaPlayer) ((WeakReference) weakThiz).get();
        if (mp == null) {
            return;
        }

        if (what == MEDIA_INFO && arg1 == MEDIA_INFO_STARTED_AS_NEXT) {
            // this acquires the wakelock if needed, and sets the client side
            // state
            mp.start();
        }
        if (mp.mEventHandler != null) {
            Message m = mp.mEventHandler.obtainMessage(what, arg1, arg2, obj);
            mp.mEventHandler.sendMessage(m);
        }
    }

    /*
     * ControlMessage
     */

    private OnControlMessageListener mOnControlMessageListener;
    public void setOnControlMessageListener(OnControlMessageListener listener) {
        mOnControlMessageListener = listener;
    }

    public static interface OnControlMessageListener {
        public String onControlResolveSegmentUrl(int segment);
    }

    @CalledByNative
    private static String onControlResolveSegmentUrl(Object weakThiz, int segment) {
        DebugLog.ifmt(TAG, "onControlResolveSegmentUrl %d", segment);
        if (weakThiz == null || !(weakThiz instanceof WeakReference<?>))
            return null;

        @SuppressWarnings("unchecked")
        WeakReference<IjkMediaPlayer> weakPlayer = (WeakReference<IjkMediaPlayer>) weakThiz;
        IjkMediaPlayer player = weakPlayer.get();
        if (player == null)
            return null;

        OnControlMessageListener listener = player.mOnControlMessageListener;
        if (listener == null)
            return null;

        return listener.onControlResolveSegmentUrl(segment);
    }

    /*
     * NativeInvoke
     */

    private OnNativeInvokeListener mOnNativeInvokeListener;
    public void setOnNativeInvokeListener(OnNativeInvokeListener listener) {
        mOnNativeInvokeListener = listener;
    }

    public static interface OnNativeInvokeListener {
        /* return whether invoke is handled */
        public boolean onNativeInvoke(int what, Bundle args);
    }

    @CalledByNative
    private static boolean onNativeInvoke(Object weakThiz, int what, Bundle args) {
        DebugLog.ifmt(TAG, "onNativeInvoke %d", what);
        if (weakThiz == null || !(weakThiz instanceof WeakReference<?>))
            return false;

        @SuppressWarnings("unchecked")
        WeakReference<IjkMediaPlayer> weakPlayer = (WeakReference<IjkMediaPlayer>) weakThiz;
        IjkMediaPlayer player = weakPlayer.get();
        if (player == null)
            return false;

        OnNativeInvokeListener listener = player.mOnNativeInvokeListener;
        if (listener == null)
            return false;

        return listener.onNativeInvoke(what, args);
    }

    /*
     * MediaCodec select
     */

    public static interface OnMediaCodecSelectListener {
        public String onMediaCodecSelect(IMediaPlayer mp, String mimeType, int profile, int level);
    }
    private OnMediaCodecSelectListener mOnMediaCodecSelectListener;
    public void setOnMediaCodecSelectListener(OnMediaCodecSelectListener listener) {
        mOnMediaCodecSelectListener = listener;
    }

    public void resetListeners() {
        super.resetListeners();
        mOnMediaCodecSelectListener = null;
    }

    @CalledByNative
    private static String onSelectCodec(Object weakThiz, String mimeType, int profile, int level) {
        if (weakThiz == null || !(weakThiz instanceof WeakReference<?>))
            return null;

        @SuppressWarnings("unchecked")
        WeakReference<IjkMediaPlayer> weakPlayer = (WeakReference<IjkMediaPlayer>) weakThiz;
        IjkMediaPlayer player = weakPlayer.get();
        if (player == null)
            return null;

        OnMediaCodecSelectListener listener = player.mOnMediaCodecSelectListener;
        if (listener == null)
            listener = DefaultMediaCodecSelector.sInstance;

        return listener.onMediaCodecSelect(player, mimeType, profile, level);
    }

    public static class DefaultMediaCodecSelector implements OnMediaCodecSelectListener {
        public static DefaultMediaCodecSelector sInstance = new DefaultMediaCodecSelector();

        @SuppressWarnings("deprecation")
        @TargetApi(Build.VERSION_CODES.JELLY_BEAN)
        public String onMediaCodecSelect(IMediaPlayer mp, String mimeType, int profile, int level) {
            if (Build.VERSION.SDK_INT < Build.VERSION_CODES.JELLY_BEAN)
                return null;

            if (TextUtils.isEmpty(mimeType))
                return null;

            Log.i(TAG, String.format(Locale.US, "onSelectCodec: mime=%s, profile=%d, level=%d", mimeType, profile, level));
            ArrayList<IjkMediaCodecInfo> candidateCodecList = new ArrayList<IjkMediaCodecInfo>();
            int numCodecs = MediaCodecList.getCodecCount();
            for (int i = 0; i < numCodecs; i++) {
                MediaCodecInfo codecInfo = MediaCodecList.getCodecInfoAt(i);
                Log.d(TAG, String.format(Locale.US, "  found codec: %s", codecInfo.getName()));
                if (codecInfo.isEncoder())
                    continue;

                String[] types = codecInfo.getSupportedTypes();
                if (types == null)
                    continue;

                for(String type: types) {
                    if (TextUtils.isEmpty(type))
                        continue;

                    Log.d(TAG, String.format(Locale.US, "    mime: %s", type));
                    if (!type.equalsIgnoreCase(mimeType))
                        continue;

                    IjkMediaCodecInfo candidate = IjkMediaCodecInfo.setupCandidate(codecInfo, mimeType);
                    if (candidate == null)
                        continue;

                    candidateCodecList.add(candidate);
                    Log.i(TAG, String.format(Locale.US, "candidate codec: %s rank=%d", codecInfo.getName(), candidate.mRank));
                    candidate.dumpProfileLevels(mimeType);
                }
            }

            if (candidateCodecList.isEmpty()) {
                return null;
            }

            IjkMediaCodecInfo bestCodec = candidateCodecList.get(0);

            for (IjkMediaCodecInfo codec : candidateCodecList) {
                if (codec.mRank > bestCodec.mRank) {
                    bestCodec = codec;
                }
            }

            if (bestCodec.mRank < IjkMediaCodecInfo.RANK_LAST_CHANCE) {
                Log.w(TAG, String.format(Locale.US, "unaccetable codec: %s", bestCodec.mCodecInfo.getName()));
                return null;
            }

            Log.i(TAG, String.format(Locale.US, "selected codec: %s rank=%d", bestCodec.mCodecInfo.getName(), bestCodec.mRank));
            return bestCodec.mCodecInfo.getName();
        }
    }

    public static native void native_profileBegin(String libName);
    public static native void native_profileEnd();
    public static native void native_setLogLevel(int level);
}
