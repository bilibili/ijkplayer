package tv.danmaku.ijk.media.player;

public abstract class AbstractMediaPlayer implements IMediaPlayer {
    protected static final int MEDIA_NOP = 0; // interface test message
    protected static final int MEDIA_PREPARED = 1;
    protected static final int MEDIA_PLAYBACK_COMPLETE = 2;
    protected static final int MEDIA_BUFFERING_UPDATE = 3;
    protected static final int MEDIA_SEEK_COMPLETE = 4;
    protected static final int MEDIA_SET_VIDEO_SIZE = 5;
    protected static final int MEDIA_TIMED_TEXT = 99;
    protected static final int MEDIA_ERROR = 100;
    protected static final int MEDIA_INFO = 200;

    protected static final int MEDIA_SET_VIDEO_SAR = 10001;
}
