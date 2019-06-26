package tv.danmaku.ijk.media.player;

public interface IjkEventListener {

    int FLUSH                   = 0;
    int ERROR                   = 100;
    int PREPARED                = 200;
    int COMPLETED               = 300;
    int VIDEO_SIZE_CHANGED      = 400;
    int SAR_CHANGED             = 401;
    int VIDEO_RENDERING_START   = 402;
    int AUDIO_RENDERING_START   = 403;
    int VIDEO_ROTATION_CHANGED  = 404;
    int BUFFERING_START         = 500;
    int BUFFERING_END           = 501;
    int BUFFERING_UPDATE        = 502;
    int PLAYBACK_STATE_CHANGED  = 700;

    void onEvent(IjkMediaPlayer mp, int what, int arg1, int arg2, Object extra);
}
