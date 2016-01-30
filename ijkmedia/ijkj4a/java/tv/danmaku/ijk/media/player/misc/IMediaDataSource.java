package tv.danmaku.ijk.media.player.misc;

@SimpleCClassName
public interface IMediaDataSource {
    int  readAt(long position, byte[] buffer, int offset, int size);
    long getSize();
    void close();
}
