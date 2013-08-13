package tv.danmaku.ijk.media.player.list;

import java.io.IOException;

import tv.danmaku.ijk.media.player.AbstractMediaPlayer;
import tv.danmaku.ijk.media.player.DebugLog;
import tv.danmaku.ijk.media.player.DummyMediaPlayer;
import tv.danmaku.ijk.media.player.SimpleMediaPlayer;
import android.content.Context;
import android.view.Surface;
import android.view.SurfaceHolder;

public abstract class AbstractMediaListPlayer extends SimpleMediaPlayer {
    private static final String TAG = AbstractMediaListPlayer.class
            .getSimpleName();
    private Context mContext;

    private SimpleMediaSegmentPlayer mItemPlayer;
    private SimpleMediaSegmentPlayer mNextPlayer;

    private SurfaceHolder mSurfaceHolder;
    private Surface mSurface;
    private Integer mWakeMode;
    private Boolean mScreenOnWhilePlaying;
    private Integer mAudioStreamType;
    private int mVideoWidth;
    private int mVideoHeight;

    private MediaList mMediaList;
    private boolean mPrepared;

    public AbstractMediaListPlayer(Context context, MediaList.Resolver resolver) {
        mContext = context.getApplicationContext();
        mItemPlayer = new SimpleMediaSegmentPlayer(0, new DummyMediaPlayer());
        try {
            mMediaList = resolver.getMediaList();
        } catch (ResolveException e) {
            DebugLog.printStackTrace(e);
        }
    }

    protected abstract AbstractMediaPlayer onCreateMediaPlayer();

    @Override
    public void setDisplay(SurfaceHolder sh) {
        mSurfaceHolder = sh;
        mSurface = null;

        mItemPlayer.setDisplay(sh);
    }

    @Override
    public void setSurface(Surface surface) {
        mSurfaceHolder = null;
        mSurface = surface;

        mItemPlayer.setSurface(surface);
    }

    @Override
    public void setDataSource(String path) throws IOException,
            IllegalArgumentException, SecurityException, IllegalStateException {
        DebugLog.wfmt(TAG, "setDataSource has no effect %s", path);
    }

    @Override
    public void prepareAsync() throws IllegalStateException {
        openPlayer(0);
    }

    @Override
    public void start() throws IllegalStateException {
        mItemPlayer.start();
    }

    @Override
    public void stop() throws IllegalStateException {
        mItemPlayer.stop();
    }

    @Override
    public void pause() throws IllegalStateException {
        mItemPlayer.pause();
    }

    @Override
    public void setWakeMode(Context context, int mode) {
        mWakeMode = mode;
    }

    @Override
    public void setScreenOnWhilePlaying(boolean screenOn) {
        mScreenOnWhilePlaying = screenOn;
    }

    @Override
    public int getVideoWidth() {
        int newWidth = mItemPlayer.getVideoWidth();
        if (newWidth > 0)
            mVideoWidth = newWidth;

        return mVideoWidth;
    }

    @Override
    public int getVideoHeight() {
        int newHeight = mItemPlayer.getVideoHeight();
        if (newHeight > 0)
            mVideoHeight = newHeight;

        return mVideoHeight;
    }

    @Override
    public boolean isPlaying() {
        return mItemPlayer.isPlaying();
    }

    @Override
    public void seekTo(long msec) throws IllegalStateException {
        if (mMediaList == null)
            return;

        MediaSegment item = mMediaList.getItemByTime(msec);
        long relativePosition = item.getRelativeTime(msec);
        if (mItemPlayer.isSameMediaItem(item)) {
            mItemPlayer.seekTo(relativePosition);
        } else if (mNextPlayer != null) {
            mNextPlayer.setSeekOnPrepared(msec);
        } else {
            mItemPlayer.releaseToDummyPlayer(true, relativePosition);
            openPlayer(item.mOrder, relativePosition);
        }
    }

    @Override
    public long getCurrentPosition() {
        return mItemPlayer.getAbsolutePosition();
    }

    @Override
    public long getDuration() {
        if (mMediaList == null)
            return 0;

        return mMediaList.getTotalDuration();
    }

    @Override
    public void release() {
        mItemPlayer.release();
        mNextPlayer.release();
    }

    @Override
    public void reset() {
        mItemPlayer.reset();
        mNextPlayer.reset();
    }

    @Override
    public void setAudioStreamType(int streamType) {
        mAudioStreamType = streamType;
    }

    private OnPreparedListener mOnItemPreparedListener = new AbstractMediaPlayer.OnPreparedListener() {
        @Override
        public void onPrepared(AbstractMediaPlayer mp) {
            onItemPrepared();
        }
    };

    private OnCompletionListener mOnItemCompletionListener = new AbstractMediaPlayer.OnCompletionListener() {
        @Override
        public void onCompletion(AbstractMediaPlayer mp) {
            onItemComplete();
        }
    };

    private void onItemPrepared() {
        SimpleMediaSegmentPlayer prevPlayer = mItemPlayer;
        mItemPlayer = mNextPlayer;
        prevPlayer.release();
        mNextPlayer = null;

        if (!mPrepared) {
            mPrepared = true;
            notifyOnPrepared(this);
        } else {
            mItemPlayer.startOnPrepared();
        }
    }

    private void onItemComplete() {
        int order = mItemPlayer.getOrder();
        if (order >= mMediaList.size()) {
            notifyOnCompletion(this);
        } else {
            mItemPlayer.releaseToDummyPlayer(true);
            openPlayer(order + 1);
        }
    }

    private void openPlayer(int order) {
        openPlayer(order, 0);
    }

    private void openPlayer(int order, long seekOnPrepared) {
        MediaSegment mediaItem = mMediaList.get(order);
        openPlayer(mediaItem, seekOnPrepared);
    }

    private void openPlayer(MediaSegment mediaItem, long seekOnPrepared) {
        if (mNextPlayer != null) {
            mNextPlayer.release();
            mNextPlayer = null;
        }
        mNextPlayer = new SimpleMediaSegmentPlayer(mediaItem.mOrder,
                onCreateMediaPlayer());

        if (mSurfaceHolder != null)
            mNextPlayer.setDisplay(mSurfaceHolder);
        else if (mSurface != null)
            mNextPlayer.setSurface(mSurface);

        if (mWakeMode != null)
            mNextPlayer.setWakeMode(mContext, mWakeMode);

        if (mScreenOnWhilePlaying != null)
            mNextPlayer.setScreenOnWhilePlaying(mScreenOnWhilePlaying);

        if (mAudioStreamType != null)
            mNextPlayer.setAudioStreamType(mAudioStreamType);

        super.attachListeners(mItemPlayer);
        mNextPlayer.setOnPreparedListener(mOnItemPreparedListener);
        mNextPlayer.setOnCompletionListener(mOnItemCompletionListener);

        mNextPlayer.setMediaSegment(mediaItem);
        mNextPlayer.setSeekOnPrepared(seekOnPrepared);
        mNextPlayer.prepareAsync();
    }
}
