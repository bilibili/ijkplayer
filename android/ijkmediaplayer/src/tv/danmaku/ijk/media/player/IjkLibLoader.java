package tv.danmaku.ijk.media.player;

public interface IjkLibLoader {
    public void loadLibrary(String libName) throws UnsatisfiedLinkError,
            SecurityException;
}
