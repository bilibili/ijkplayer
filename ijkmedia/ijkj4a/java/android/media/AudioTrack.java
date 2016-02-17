package android.media;

@SimpleCClassName
@IncludeUtil
public class AudioTrack {
    public AudioTrack(int streamType, int sampleRateInHz, int channelConfig, int audioFormat, int bufferSizeInBytes, int mode);

    public static int   getMinBufferSize(int sampleRateInHz, int channelConfig, int audioFormat);
    public static float getMaxVolume();
    public static float getMinVolume();
    public static int   getNativeOutputSampleRate (int streamType);

    public void play();
    public void pause();
    public void stop();
    public void flush();
    public void release();

    public int write(byte[] audioData, int offsetInBytes, int sizeInBytes);

    public int setStereoVolume(float leftGain, float rightGain);
    public int getAudioSessionId();

    @MinApi(23)
    public PlaybackParams getPlaybackParams();
    @MinApi(23)
    void setPlaybackParams(PlaybackParams params);
}
