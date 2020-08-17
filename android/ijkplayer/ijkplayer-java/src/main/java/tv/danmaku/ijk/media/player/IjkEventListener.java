package tv.danmaku.ijk.media.player;

public interface IjkEventListener {

    int FLUSH                       = 0;
    int ERROR                       = 100;
    int PREPARED                    = 200;
    int COMPLETED                   = 300;
    int VIDEO_SIZE_CHANGED          = 400;
    int SAR_CHANGED                 = 401;
    int VIDEO_RENDERING_START       = 402;
    int AUDIO_RENDERING_START       = 403;
    int VIDEO_ROTATION_CHANGED      = 404;
    int AUDIO_DECODED_START         = 405;
    int VIDEO_DECODED_START         = 406;
    int OPEN_INPUT                  = 407;
    int FIND_STREAM_INFO            = 408;
    int COMPONENT_OPEN              = 409;
    int VIDEO_SEEK_RENDERING_START  = 410;
    int AUDIO_SEEK_RENDERING_START  = 411;

    int BUFFERING_START             = 500;
    int BUFFERING_END               = 501;
    int BUFFERING_UPDATE            = 502;
    int BUFFERING_BYTES_UPDATE      = 503;
    int BUFFERING_TIME_UPDATE       = 504;
    int CURRENT_POSITION_UPDATE     = 510;
    
    int SEEK_COMPLETE               = 600;
    int PLAYBACK_STATE_CHANGED      = 700;
    int TIMED_TEXT                  = 800;
    int ACCURATE_SEEK_COMPLETE      = 900;
    int GET_IMG_STATE               = 1000;



    void onEvent(IjkMediaPlayer mp, int what, int arg1, int arg2, Object extra);
}
