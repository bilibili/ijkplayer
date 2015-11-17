/*
 * Copyright (C) 2015 Zhang Rui <bbcallen@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package tv.danmaku.ijk.media.sample.fragments;

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v4.app.Fragment;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.TextView;

import java.util.Locale;

import tv.danmaku.ijk.media.player.misc.ITrackInfo;
import tv.danmaku.ijk.media.sample.R;

public class TracksFragment extends Fragment {
    private ListView mTrackListView;
    private TrackAdapter mAdapter;

    public static TracksFragment newInstance() {
        TracksFragment f = new TracksFragment();
        return f;
    }

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        ViewGroup viewGroup = (ViewGroup) inflater.inflate(R.layout.fragment_track_list, container, false);
        mTrackListView = (ListView) viewGroup.findViewById(R.id.track_list_view);
        return viewGroup;
    }

    @Override
    public void onActivityCreated(@Nullable Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);

        final Activity activity = getActivity();

        mAdapter = new TrackAdapter(activity);
        mTrackListView.setAdapter(mAdapter);

        if (activity instanceof ITrackHolder) {
            final ITrackHolder trackHolder = (ITrackHolder) activity;
            mAdapter.setTrackHolder(trackHolder);

            int selectedVideoTrack = trackHolder.getSelectedTrack(ITrackInfo.MEDIA_TRACK_TYPE_VIDEO);
            int selectedAudioTrack = trackHolder.getSelectedTrack(ITrackInfo.MEDIA_TRACK_TYPE_AUDIO);
            if (selectedVideoTrack >= 0)
                mTrackListView.setItemChecked(selectedVideoTrack, true);
            if (selectedAudioTrack >= 0)
                mTrackListView.setItemChecked(selectedAudioTrack, true);

            mTrackListView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
                @Override
                public void onItemClick(AdapterView<?> parent, View view, final int position, final long id) {
                    TrackItem trackItem = (TrackItem) mTrackListView.getItemAtPosition(position);
                    for (int i = 0; i < mAdapter.getCount(); ++i) {
                        TrackItem compareItem = mAdapter.getItem(i);
                        if (compareItem.mIndex == trackItem.mIndex)
                            continue;

                        if (compareItem.mTrackInfo.getTrackType() != trackItem.mTrackInfo.getTrackType())
                            continue;

                        if (mTrackListView.isItemChecked(i))
                            mTrackListView.setItemChecked(i, false);
                    }
                    if (mTrackListView.isItemChecked(position)) {
                        trackHolder.selectTrack(trackItem.mIndex);
                    } else {
                        trackHolder.deselectTrack(trackItem.mIndex);
                    }
                }
            });
        } else {
            Log.e("TracksFragment", "activity is not an instance of ITrackHolder.");
        }
    }

    public interface ITrackHolder {
        ITrackInfo[] getTrackInfo();
        int getSelectedTrack(int trackType);
        void selectTrack(int stream);
        void deselectTrack(int stream);
    }

    final class TrackItem {
        public int mIndex;
        public ITrackInfo mTrackInfo;

        public String mInfoInline;

        public TrackItem(int index, ITrackInfo trackInfo) {
            mIndex = index;
            mTrackInfo = trackInfo;
            mInfoInline = String.format(Locale.US, "# %d: %s", mIndex, mTrackInfo.getInfoInline());
        }

        public String getInfoInline() {
            return mInfoInline;
        }
    }

    final class TrackAdapter extends ArrayAdapter<TrackItem> {
        private ITrackHolder mTrackHolder;
        private ITrackInfo[] mTrackInfos;

        public TrackAdapter(Context context) {
            super(context, android.R.layout.simple_list_item_checked);
        }

        public void setTrackHolder(ITrackHolder trackHolder) {
            clear();
            mTrackHolder = trackHolder;
            mTrackInfos = mTrackHolder.getTrackInfo();
            if (mTrackInfos != null) {
                for(ITrackInfo trackInfo: mTrackInfos) {
                    int index = getCount();
                    TrackItem item = new TrackItem(index, trackInfo);
                    add(item);
                }
            }
        }

        @Override
        public long getItemId(int position) {
            return position;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            View view = convertView;
            if (view == null) {
                LayoutInflater inflater = LayoutInflater.from(parent.getContext());
                view = inflater.inflate(android.R.layout.simple_list_item_checked, parent, false);
            }

            ViewHolder viewHolder = (ViewHolder) view.getTag();
            if (viewHolder == null) {
                viewHolder = new ViewHolder();
                viewHolder.mNameTextView = (TextView) view.findViewById(android.R.id.text1);
            }

            TrackItem item = getItem(position);
            viewHolder.mNameTextView.setText(item.getInfoInline());

            return view;
        }

        final class ViewHolder {
            public TextView mNameTextView;
        }
    }
}
