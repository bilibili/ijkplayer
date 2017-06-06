package android.media;

import java.nio.ByteBuffer;

@SimpleCClassName
@MinApi(16)
public class MediaFormat {
    public MediaFormat();

    public final static MediaFormat createVideoFormat(String mime, int width, int height);

    public final int   getInteger(String name);
    public final void  setInteger(String name, int value);
    public final void  setByteBuffer(String name, ByteBuffer bytes);
}
