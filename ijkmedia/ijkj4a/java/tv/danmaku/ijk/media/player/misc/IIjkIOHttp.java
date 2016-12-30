package tv.danmaku.ijk.media.player.misc;

@SimpleCClassName
public interface IIjkIOHttp {
    int  open();
    int  read(byte[] buffer, int size);
    long seek(long offset, int whence);
    int  close();
}