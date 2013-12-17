package tv.danmaku.ijk.media.player.list;

import java.util.ArrayList;

import android.content.Context;

public final class MediaList {
    private ArrayList<MediaSegment> mSegmentList = new ArrayList<MediaSegment>();

    private long mDuration;

    public MediaList() {
    }

    public MediaList(MediaSegment mediaSegment) {
        add(mediaSegment);
    }

    public void add(MediaSegment mediaSegment) {
        mediaSegment.mOrder = mSegmentList.size();
        mediaSegment.mStartTime = mDuration;
        mDuration += mediaSegment.mDuration;
        mSegmentList.add(mediaSegment);
    }

    public long getTotalDuration() {
        return mDuration;
    }

    public MediaSegment get(int index) {
        return mSegmentList.get(index);
    }

    public int size() {
        return mSegmentList.size();
    }

    public void clear() {
        mSegmentList.clear();
        mDuration = 0;
    }

    public MediaSegment getItemByTime(long position) {
        MediaSegment lowerItem = null;
        for (MediaSegment item : mSegmentList) {
            if (item.mStartTime > position)
                break;

            lowerItem = item;
        }

        return lowerItem;
    }

    public static interface Resolver {
        public abstract void clearCache();

        public abstract void resolve(Context context)throws ResolveException;

        public abstract void resolveAsync(Context context);

        public abstract MediaList getMediaList() ;
        
        /**
         * Youku's resolution
         * <pre>
         * 
         * hd3|hd2|mp4|flv
         * ---------------
         *  3 | 2 | 1 | 0
         * </pre>
         * 
         * @param resolution
         *            video resolution
         * @throws ResolveException
         */
        MediaList getMediaList(int resolution);

        public abstract MediaSegment getMediaSegment(int segmentId);

        void setOnResolvedListener(OnResolvedListener l);
    }

    public static class SimpleResolver implements Resolver {
        private MediaList mMediaList;

        public SimpleResolver(MediaList mediaList) {
            mMediaList = mediaList;
        }

        @Override
        public void clearCache() {
            // do nothing
        }

        @Override
        public void resolve(Context context) {
            // do nothing
        }

        @Override
        public void resolveAsync(Context context) {
            // do nothing
        }

        @Override
        public MediaList getMediaList(){
            return mMediaList;
        }

        @Override
        public MediaSegment getMediaSegment(int segmentId) {
            return mMediaList.mSegmentList.get(segmentId);
        }

        @Override
        public void setOnResolvedListener(OnResolvedListener l) {
            // do nothing
        }

        @Override
        public MediaList getMediaList(int mode){
            
            return mMediaList;
        }
    }
    public interface OnResolvedListener{
        void onResolved(Resolver resolver);
    }
}
