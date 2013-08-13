package tv.danmaku.ijk.media.player.list;

import tv.danmaku.ijk.media.player.AbstractMediaPlayer;
import tv.danmaku.ijk.media.player.IjkMediaPlayer;
import tv.danmaku.ijk.media.player.list.MediaList.Resolver;
import android.content.Context;

public class IjkMediaListPlayer extends AbstractMediaListPlayer {
    public IjkMediaListPlayer(Context context, Resolver resolver) {
        super(context, resolver);
    }

    @Override
    protected AbstractMediaPlayer onCreateMediaPlayer() {
        return new IjkMediaPlayer();
    }
}
