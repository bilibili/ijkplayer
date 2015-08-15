package tv.danmaku.ijk.media.widget;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.graphics.SurfaceTexture;
import android.media.AudioManager;
import android.net.Uri;
import android.util.AttributeSet;
import android.util.Pair;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.TextureView;
import android.view.View;
import android.view.ViewGroup;

import java.io.IOException;
import java.util.List;

import tv.danmaku.ijk.media.player.IMediaPlayer;
import tv.danmaku.ijk.media.player.IjkMediaPlayer;
import tv.danmaku.ijk.media.player.option.AvFourCC;
import tv.danmaku.ijk.media.player.option.format.AvFormatOption_HttpDetectRangeSupport;
import tv.danmaku.ijk.media.sample.R;

/**
 * Created by dima on 8/5/15.
 */
public class VideoViewTextureView extends TextureView implements
        MediaController.MediaPlayerControl {
    private static final String TAG = VideoView.class.getName();

    private Uri mUri;
    private long mDuration;
    private String mUserAgent;

    private static final int STATE_ERROR = -1;
    private static final int STATE_IDLE = 0;
    private static final int STATE_PREPARING = 1;
    private static final int STATE_PREPARED = 2;
    private static final int STATE_PLAYING = 3;
    private static final int STATE_PAUSED = 4;
    private static final int STATE_PLAYBACK_COMPLETED = 5;
    private static final int STATE_SUSPEND = 6;
    private static final int STATE_RESUME = 7;
    private static final int STATE_SUSPEND_UNSUPPORTED = 8;

    private int mCurrentState = STATE_IDLE;
    private int mTargetState = STATE_IDLE;

    private int mVideoLayout = VIDEO_LAYOUT_SCALE;
    public static final int VIDEO_LAYOUT_ORIGIN = 0;
    public static final int VIDEO_LAYOUT_SCALE = 1;
    public static final int VIDEO_LAYOUT_STRETCH = 2;
    public static final int VIDEO_LAYOUT_ZOOM = 3;

    //TODO CHANGED TextureView doesn't use SurfaceHold, instead of it uses SurfaceTexture
//    private SurfaceHolder mSurfaceHolder = null;
    private SurfaceTexture mSurfaceTexture = null;

    private IMediaPlayer mMediaPlayer = null;
    private int mVideoWidth;
    private int mVideoHeight;
    private int mVideoSarNum;
    private int mVideoSarDen;
    private int mSurfaceWidth;
    private int mSurfaceHeight;
    private MediaController mMediaController;
    private View mMediaBufferingIndicator;
    private IMediaPlayer.OnCompletionListener mOnCompletionListener;
    private IMediaPlayer.OnPreparedListener mOnPreparedListener;
    private IMediaPlayer.OnErrorListener mOnErrorListener;
    private IMediaPlayer.OnSeekCompleteListener mOnSeekCompleteListener;
    private IMediaPlayer.OnInfoListener mOnInfoListener;
    private IMediaPlayer.OnBufferingUpdateListener mOnBufferingUpdateListener;
    private int mCurrentBufferPercentage;
    private long mSeekWhenPrepared;
    private boolean mCanPause = true;
    private boolean mCanSeekBack = true;
    private boolean mCanSeekForward = true;
    private Context mContext;

    public VideoViewTextureView(Context context) {
        super(context);
        initVideoView(context);
    }

    public VideoViewTextureView(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public VideoViewTextureView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        initVideoView(context);
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        int width = getDefaultSize(mVideoWidth, widthMeasureSpec);
        int height = getDefaultSize(mVideoHeight, heightMeasureSpec);
        setMeasuredDimension(width, height);
    }

    /**
     * Set the display options
     *
     * @param layout
     *            <ul>
     *            <li>{@link #VIDEO_LAYOUT_ORIGIN}
     *            <li>{@link #VIDEO_LAYOUT_SCALE}
     *            <li>{@link #VIDEO_LAYOUT_STRETCH}
     *            <li>{@link #VIDEO_LAYOUT_ZOOM}
     *            </ul>
     * @param aspectRatio
     *            video aspect ratio, will audo detect if 0.
     */
    public void setVideoLayout(int layout) {
        ViewGroup.LayoutParams lp = getLayoutParams();
        Pair<Integer, Integer> res  = ScreenResolution.getResolution(mContext);
        int windowWidth = res.first.intValue(), windowHeight = res.second.intValue();
        float windowRatio = windowWidth / (float) windowHeight;
        int sarNum = mVideoSarNum;
        int sarDen = mVideoSarDen;
        if (mVideoHeight > 0 && mVideoWidth > 0) {
            float videoRatio = ((float) (mVideoWidth)) / mVideoHeight;
            if (sarNum > 0 && sarDen > 0)
                videoRatio = videoRatio * sarNum / sarDen;
            mSurfaceHeight = mVideoHeight;
            mSurfaceWidth = mVideoWidth;

            if (VIDEO_LAYOUT_ORIGIN == layout && mSurfaceWidth < windowWidth
                    && mSurfaceHeight < windowHeight) {
                lp.width = (int) (mSurfaceHeight * videoRatio);
                lp.height = mSurfaceHeight;
            } else if (layout == VIDEO_LAYOUT_ZOOM) {
                lp.width = windowRatio > videoRatio ? windowWidth
                        : (int) (videoRatio * windowHeight);
                lp.height = windowRatio < videoRatio ? windowHeight
                        : (int) (windowWidth / videoRatio);
            } else {
                boolean full = layout == VIDEO_LAYOUT_STRETCH;
                lp.width = (full || windowRatio < videoRatio) ? windowWidth
                        : (int) (videoRatio * windowHeight);
                lp.height = (full || windowRatio > videoRatio) ? windowHeight
                        : (int) (windowWidth / videoRatio);
            }
            setLayoutParams(lp);
//            getHolder().setFixedSize(mSurfaceWidth, mSurfaceHeight);
            DebugLog.dfmt(
                    TAG,
                    "VIDEO: %dx%dx%f[SAR:%d:%d], Surface: %dx%d, LP: %dx%d, Window: %dx%dx%f",
                    mVideoWidth, mVideoHeight, videoRatio, mVideoSarNum,
                    mVideoSarDen, mSurfaceWidth, mSurfaceHeight, lp.width,
                    lp.height, windowWidth, windowHeight, windowRatio);
        }
        mVideoLayout = layout;
    }

    private void initVideoView(Context ctx) {
        mContext = ctx;
        mVideoWidth = 0;
        mVideoHeight = 0;
        mVideoSarNum = 0;
        mVideoSarDen = 0;

        //TODO CHANGED It was changed because TextureView uses other callback
        setSurfaceTextureListener(mTVCallback);
//        getHolder().addCallback(mSHCallback);
        setFocusable(true);
        setFocusableInTouchMode(true);
        requestFocus();
        mCurrentState = STATE_IDLE;
        mTargetState = STATE_IDLE;
        if (ctx instanceof Activity)
            ((Activity) ctx).setVolumeControlStream(AudioManager.STREAM_MUSIC);
    }

    public boolean isValid() {
        return (mSurfaceTexture != null);
    }

    public void setVideoPath(String path) {
        setVideoURI(Uri.parse(path));
    }

    public void setVideoURI(Uri uri) {
        mUri = uri;
        mSeekWhenPrepared = 0;
        openVideo();
        requestLayout();
        invalidate();
    }

    public void setUserAgent(String ua) {
        mUserAgent = ua;
    }

    public void stopPlayback() {
        if (mMediaPlayer != null) {
            mMediaPlayer.stop();
            mMediaPlayer.release();
            mMediaPlayer = null;
            mCurrentState = STATE_IDLE;
            mTargetState = STATE_IDLE;
        }
    }

    private void openVideo() {
        if (mUri == null || mSurfaceTexture == null)
            return;

        Intent i = new Intent("com.android.music.musicservicecommand");
        i.putExtra("command", "pause");
        mContext.sendBroadcast(i);

        release(false);
        try {
            mDuration = -1;
            mCurrentBufferPercentage = 0;
            // mMediaPlayer = new AndroidMediaPlayer();
            IjkMediaPlayer ijkMediaPlayer = null;
            if (mUri != null) {
                ijkMediaPlayer = new IjkMediaPlayer();
                ijkMediaPlayer.setAvOption(AvFormatOption_HttpDetectRangeSupport.Disable);
                ijkMediaPlayer.setOverlayFormat(AvFourCC.SDL_FCC_RV32);

                ijkMediaPlayer.setAvCodecOption("skip_loop_filter", "48");
                ijkMediaPlayer.setFrameDrop(12);
                if (mUserAgent != null) {
                    ijkMediaPlayer.setAvFormatOption("user_agent", mUserAgent);
                }
            }
            mMediaPlayer = ijkMediaPlayer;
            mMediaPlayer.setOnPreparedListener(mPreparedListener);
            mMediaPlayer.setOnVideoSizeChangedListener(mSizeChangedListener);
            mMediaPlayer.setOnCompletionListener(mCompletionListener);
            mMediaPlayer.setOnErrorListener(mErrorListener);
            mMediaPlayer.setOnBufferingUpdateListener(mBufferingUpdateListener);
            mMediaPlayer.setOnInfoListener(mInfoListener);
            mMediaPlayer.setOnSeekCompleteListener(mSeekCompleteListener);
            if (mUri != null)
                mMediaPlayer.setDataSource(mUri.toString());
            //TODO CHANGED Create surface to show data from MediaPlayer
//            mMediaPlayer.setDisplay(mSurfaceHolder);
            mMediaPlayer.setSurface(new Surface(mSurfaceTexture));
            mMediaPlayer.setScreenOnWhilePlaying(true);
            mMediaPlayer.prepareAsync();
            mCurrentState = STATE_PREPARING;
            attachMediaController();
        } catch (IOException ex) {
            DebugLog.e(TAG, "Unable to open content: " + mUri, ex);
            mCurrentState = STATE_ERROR;
            mTargetState = STATE_ERROR;
            mErrorListener.onError(mMediaPlayer,
                    IMediaPlayer.MEDIA_ERROR_UNKNOWN, 0);
            return;
        } catch (IllegalArgumentException ex) {
            DebugLog.e(TAG, "Unable to open content: " + mUri, ex);
            mCurrentState = STATE_ERROR;
            mTargetState = STATE_ERROR;
            mErrorListener.onError(mMediaPlayer,
                    IMediaPlayer.MEDIA_ERROR_UNKNOWN, 0);
            return;
        }
    }

    public void setMediaController(MediaController controller) {
        if (mMediaController != null)
            mMediaController.hide();
        mMediaController = controller;
        attachMediaController();
    }

    public void setMediaBufferingIndicator(View mediaBufferingIndicator) {
        if (mMediaBufferingIndicator != null)
            mMediaBufferingIndicator.setVisibility(View.GONE);
        mMediaBufferingIndicator = mediaBufferingIndicator;
    }

    private void attachMediaController() {
        if (mMediaPlayer != null && mMediaController != null) {
            mMediaController.setMediaPlayer(this);
            View anchorView = this.getParent() instanceof View ? (View) this
                    .getParent() : this;
            mMediaController.setAnchorView(anchorView);
            mMediaController.setEnabled(isInPlaybackState());

            if (mUri != null) {
                List<String> paths = mUri.getPathSegments();
                String name = paths == null || paths.isEmpty() ? "null" : paths
                        .get(paths.size() - 1);
                mMediaController.setFileName(name);
            }
        }
    }

    IMediaPlayer.OnVideoSizeChangedListener mSizeChangedListener = new IMediaPlayer.OnVideoSizeChangedListener() {
        public void onVideoSizeChanged(IMediaPlayer mp, int width, int height,
                                       int sarNum, int sarDen) {
            DebugLog.dfmt(TAG, "onVideoSizeChanged: (%dx%d)", width, height);
            mVideoWidth = mp.getVideoWidth();
            mVideoHeight = mp.getVideoHeight();
            mVideoSarNum = sarNum;
            mVideoSarDen = sarDen;
            if (mVideoWidth != 0 && mVideoHeight != 0)
                setVideoLayout(mVideoLayout);
        }
    };

    IMediaPlayer.OnPreparedListener mPreparedListener = new IMediaPlayer.OnPreparedListener() {
        public void onPrepared(IMediaPlayer mp) {
            DebugLog.d(TAG, "onPrepared");
            mCurrentState = STATE_PREPARED;
            mTargetState = STATE_PLAYING;

            if (mOnPreparedListener != null)
                mOnPreparedListener.onPrepared(mMediaPlayer);
            if (mMediaController != null)
                mMediaController.setEnabled(true);
            mVideoWidth = mp.getVideoWidth();
            mVideoHeight = mp.getVideoHeight();

            long seekToPosition = mSeekWhenPrepared;

            if (seekToPosition != 0)
                seekTo(seekToPosition);
            if (mVideoWidth != 0 && mVideoHeight != 0) {
                setVideoLayout(mVideoLayout);
                if (mSurfaceWidth == mVideoWidth
                        && mSurfaceHeight == mVideoHeight) {
                    if (mTargetState == STATE_PLAYING) {
                        start();
                        if (mMediaController != null)
                            mMediaController.show();
                    } else if (!isPlaying()
                            && (seekToPosition != 0 || getCurrentPosition() > 0)) {
                        if (mMediaController != null)
                            mMediaController.show(0);
                    }
                }
            } else if (mTargetState == STATE_PLAYING) {
                start();
            }
        }
    };

    private IMediaPlayer.OnCompletionListener mCompletionListener = new IMediaPlayer.OnCompletionListener() {
        public void onCompletion(IMediaPlayer mp) {
            DebugLog.d(TAG, "onCompletion");
            mCurrentState = STATE_PLAYBACK_COMPLETED;
            mTargetState = STATE_PLAYBACK_COMPLETED;
            if (mMediaController != null)
                mMediaController.hide();
            if (mOnCompletionListener != null)
                mOnCompletionListener.onCompletion(mMediaPlayer);
        }
    };

    private IMediaPlayer.OnErrorListener mErrorListener = new IMediaPlayer.OnErrorListener() {
        public boolean onError(IMediaPlayer mp, int framework_err, int impl_err) {
            DebugLog.dfmt(TAG, "Error: %d, %d", framework_err, impl_err);
            mCurrentState = STATE_ERROR;
            mTargetState = STATE_ERROR;
            if (mMediaController != null)
                mMediaController.hide();

            if (mOnErrorListener != null) {
                if (mOnErrorListener.onError(mMediaPlayer, framework_err,
                        impl_err))
                    return true;
            }

            if (getWindowToken() != null) {
                int message = framework_err == IMediaPlayer.MEDIA_ERROR_NOT_VALID_FOR_PROGRESSIVE_PLAYBACK ? R.string.vitamio_videoview_error_text_invalid_progressive_playback
                        : R.string.vitamio_videoview_error_text_unknown;

                new AlertDialog.Builder(mContext)
                        .setTitle(R.string.vitamio_videoview_error_title)
                        .setMessage(message)
                        .setPositiveButton(
                                R.string.vitamio_videoview_error_button,
                                new DialogInterface.OnClickListener() {
                                    public void onClick(DialogInterface dialog,
                                                        int whichButton) {
                                        if (mOnCompletionListener != null)
                                            mOnCompletionListener
                                                    .onCompletion(mMediaPlayer);
                                    }
                                }).setCancelable(false).show();
            }
            return true;
        }
    };

    private IMediaPlayer.OnBufferingUpdateListener mBufferingUpdateListener = new IMediaPlayer.OnBufferingUpdateListener() {
        public void onBufferingUpdate(IMediaPlayer mp, int percent) {
            mCurrentBufferPercentage = percent;
            if (mOnBufferingUpdateListener != null)
                mOnBufferingUpdateListener.onBufferingUpdate(mp, percent);
        }
    };

    private IMediaPlayer.OnInfoListener mInfoListener = new IMediaPlayer.OnInfoListener() {
        @Override
        public boolean onInfo(IMediaPlayer mp, int what, int extra) {
            DebugLog.dfmt(TAG, "onInfo: (%d, %d)", what, extra);
            if (mOnInfoListener != null) {
                mOnInfoListener.onInfo(mp, what, extra);
            } else if (mMediaPlayer != null) {
                if (what == IMediaPlayer.MEDIA_INFO_BUFFERING_START) {
                    DebugLog.dfmt(TAG, "onInfo: (MEDIA_INFO_BUFFERING_START)");
                    if (mMediaBufferingIndicator != null)
                        mMediaBufferingIndicator.setVisibility(View.VISIBLE);
                } else if (what == IMediaPlayer.MEDIA_INFO_BUFFERING_END) {
                    DebugLog.dfmt(TAG, "onInfo: (MEDIA_INFO_BUFFERING_END)");
                    if (mMediaBufferingIndicator != null)
                        mMediaBufferingIndicator.setVisibility(View.GONE);
                }
            }

            return true;
        }
    };

    private IMediaPlayer.OnSeekCompleteListener mSeekCompleteListener = new IMediaPlayer.OnSeekCompleteListener() {
        @Override
        public void onSeekComplete(IMediaPlayer mp) {
            DebugLog.d(TAG, "onSeekComplete");
            if (mOnSeekCompleteListener != null)
                mOnSeekCompleteListener.onSeekComplete(mp);
        }
    };

    public void setOnPreparedListener(IMediaPlayer.OnPreparedListener l) {
        mOnPreparedListener = l;
    }

    public void setOnCompletionListener(IMediaPlayer.OnCompletionListener l) {
        mOnCompletionListener = l;
    }

    public void setOnErrorListener(IMediaPlayer.OnErrorListener l) {
        mOnErrorListener = l;
    }

    public void setOnBufferingUpdateListener(IMediaPlayer.OnBufferingUpdateListener l) {
        mOnBufferingUpdateListener = l;
    }

    public void setOnSeekCompleteListener(IMediaPlayer.OnSeekCompleteListener l) {
        mOnSeekCompleteListener = l;
    }

    public void setOnInfoListener(IMediaPlayer.OnInfoListener l) {
        mOnInfoListener = l;
    }

    //TODO CHANGED New callback for TextureView. Almost the same code except setDisplay use setSurface
    TextureView.SurfaceTextureListener mTVCallback = new TextureView.SurfaceTextureListener() {

        @Override
        public void onSurfaceTextureAvailable(SurfaceTexture surface, int width, int height) {
            mSurfaceTexture = surface;
            if (mMediaPlayer != null && mCurrentState == STATE_SUSPEND
                    && mTargetState == STATE_RESUME) {
                mMediaPlayer.setSurface(new Surface(mSurfaceTexture));
                resume();
            } else {
                openVideo();
            }
        }

        @Override
        public void onSurfaceTextureSizeChanged(SurfaceTexture surface, int w, int h) {
            mSurfaceTexture = surface;
            if (mMediaPlayer != null) {
                mMediaPlayer.setSurface(new Surface(mSurfaceTexture));
            }

            mSurfaceWidth = w;
            mSurfaceHeight = h;
            boolean isValidState = (mTargetState == STATE_PLAYING);
            boolean hasValidSize = (mVideoWidth == w && mVideoHeight == h);
            if (mMediaPlayer != null && isValidState && hasValidSize) {
                if (mSeekWhenPrepared != 0)
                    seekTo(mSeekWhenPrepared);
                start();
                if (mMediaController != null) {
                    if (mMediaController.isShowing())
                        mMediaController.hide();
                    mMediaController.show();
                }
            }

        }

        @Override
        public boolean onSurfaceTextureDestroyed(SurfaceTexture surface) {
            mSurfaceTexture = null;
            if (mMediaController != null)
                mMediaController.hide();
            if (mCurrentState != STATE_SUSPEND)
                release(true);
            return false;
        }

        @Override
        public void onSurfaceTextureUpdated(SurfaceTexture surface) {

        }
    };

//    SurfaceHolder.Callback mSHCallback = new SurfaceHolder.Callback() {
//        public void surfaceChanged(SurfaceHolder holder, int format, int w,
//                                   int h) {
//            mSurfaceHolder = holder;
//            if (mMediaPlayer != null) {
//                mMediaPlayer.setDisplay(mSurfaceHolder);
//            }
//
//            mSurfaceWidth = w;
//            mSurfaceHeight = h;
//            boolean isValidState = (mTargetState == STATE_PLAYING);
//            boolean hasValidSize = (mVideoWidth == w && mVideoHeight == h);
//            if (mMediaPlayer != null && isValidState && hasValidSize) {
//                if (mSeekWhenPrepared != 0)
//                    seekTo(mSeekWhenPrepared);
//                start();
//                if (mMediaController != null) {
//                    if (mMediaController.isShowing())
//                        mMediaController.hide();
//                    mMediaController.show();
//                }
//            }
//        }
//
//        public void surfaceCreated(SurfaceHolder holder) {
//            mSurfaceHolder = holder;
//            if (mMediaPlayer != null && mCurrentState == STATE_SUSPEND
//                    && mTargetState == STATE_RESUME) {
//                mMediaPlayer.setDisplay(mSurfaceHolder);
//                resume();
//            } else {
//                openVideo();
//            }
//        }
//
//        public void surfaceDestroyed(SurfaceHolder holder) {
//            mSurfaceHolder = null;
//            if (mMediaController != null)
//                mMediaController.hide();
//            if (mCurrentState != STATE_SUSPEND)
//                release(true);
//        }
//    };

    private void release(boolean cleartargetstate) {
        if (mMediaPlayer != null) {
            mMediaPlayer.reset();
            mMediaPlayer.release();
            mMediaPlayer = null;
            mCurrentState = STATE_IDLE;
            if (cleartargetstate)
                mTargetState = STATE_IDLE;
        }
    }

    @Override
    public boolean onTouchEvent(MotionEvent ev) {
        if (isInPlaybackState() && mMediaController != null)
            toggleMediaControlsVisiblity();
        return false;
    }

    @Override
    public boolean onTrackballEvent(MotionEvent ev) {
        if (isInPlaybackState() && mMediaController != null)
            toggleMediaControlsVisiblity();
        return false;
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        boolean isKeyCodeSupported = keyCode != KeyEvent.KEYCODE_BACK
                && keyCode != KeyEvent.KEYCODE_VOLUME_UP
                && keyCode != KeyEvent.KEYCODE_VOLUME_DOWN
                && keyCode != KeyEvent.KEYCODE_MENU
                && keyCode != KeyEvent.KEYCODE_CALL
                && keyCode != KeyEvent.KEYCODE_ENDCALL;
        if (isInPlaybackState() && isKeyCodeSupported
                && mMediaController != null) {
            if (keyCode == KeyEvent.KEYCODE_HEADSETHOOK
                    || keyCode == KeyEvent.KEYCODE_MEDIA_PLAY_PAUSE
                    || keyCode == KeyEvent.KEYCODE_SPACE) {
                if (mMediaPlayer.isPlaying()) {
                    pause();
                    mMediaController.show();
                } else {
                    start();
                    mMediaController.hide();
                }
                return true;
            } else if (keyCode == KeyEvent.KEYCODE_MEDIA_STOP
                    && mMediaPlayer.isPlaying()) {
                pause();
                mMediaController.show();
            } else {
                toggleMediaControlsVisiblity();
            }
        }

        return super.onKeyDown(keyCode, event);
    }

    private void toggleMediaControlsVisiblity() {
        if (mMediaController.isShowing()) {
            mMediaController.hide();
        } else {
            mMediaController.show();
        }
    }

    @Override
    public void start() {
        if (isInPlaybackState()) {
            mMediaPlayer.start();
            mCurrentState = STATE_PLAYING;
        }
        mTargetState = STATE_PLAYING;
    }

    @Override
    public void pause() {
        if (isInPlaybackState()) {
            if (mMediaPlayer.isPlaying()) {
                mMediaPlayer.pause();
                mCurrentState = STATE_PAUSED;
            }
        }
        mTargetState = STATE_PAUSED;
    }

    public void resume() {
        //TODO CHANGED use SurfaceTexture istead of SurfaceHolde
        if (mSurfaceTexture == null && mCurrentState == STATE_SUSPEND) {
            mTargetState = STATE_RESUME;
        } else if (mCurrentState == STATE_SUSPEND_UNSUPPORTED) {
            openVideo();
        }
    }

    @Override
    public int getDuration() {
        if (isInPlaybackState()) {
            if (mDuration > 0)
                return (int) mDuration;
            mDuration = mMediaPlayer.getDuration();
            return (int) mDuration;
        }
        mDuration = -1;
        return (int) mDuration;
    }

    @Override
    public int getCurrentPosition() {
        if (isInPlaybackState()) {
            long position = mMediaPlayer.getCurrentPosition();
            return (int) position;
        }
        return 0;
    }

    @Override
    public void seekTo(long msec) {
        if (isInPlaybackState()) {
            mMediaPlayer.seekTo(msec);
            mSeekWhenPrepared = 0;
        } else {
            mSeekWhenPrepared = msec;
        }
    }

    @Override
    public boolean isPlaying() {
        return isInPlaybackState() && mMediaPlayer.isPlaying();
    }

    @Override
    public int getBufferPercentage() {
        if (mMediaPlayer != null)
            return mCurrentBufferPercentage;
        return 0;
    }

    public int getVideoWidth() {
        return mVideoWidth;
    }

    public int getVideoHeight() {
        return mVideoHeight;
    }

    protected boolean isInPlaybackState() {
        return (mMediaPlayer != null && mCurrentState != STATE_ERROR
                && mCurrentState != STATE_IDLE && mCurrentState != STATE_PREPARING);
    }

    public boolean canPause() {
        return mCanPause;
    }

    public boolean canSeekBackward() {
        return mCanSeekBack;
    }

    public boolean canSeekForward() {
        return mCanSeekForward;
    }
}
