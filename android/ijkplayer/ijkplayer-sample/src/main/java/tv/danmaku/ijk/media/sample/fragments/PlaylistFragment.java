package tv.danmaku.ijk.media.sample.fragments;

import android.graphics.Color;
import android.os.Bundle;
import android.support.v4.app.ListFragment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ListView;

import tv.danmaku.ijk.media.sample.activities.MainActivity;
import tv.danmaku.ijk.media.sample.content.MediaBean;

/**
 * A fragment representing a list of Items.
 * <p/>
 * <p/>
 */
public class PlaylistFragment extends ListFragment {
    public static final String TAG = PlaylistFragment.class.getSimpleName();
    private ArrayAdapter<MediaBean> mAdapter;

    public static PlaylistFragment newInstance() {
        PlaylistFragment fragment = new PlaylistFragment();
        return fragment;
    }

    /**
     * Mandatory empty constructor for the fragment manager to instantiate the
     * fragment (e.g. upon screen orientation changes).
     */
    public PlaylistFragment() {
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mAdapter = new ArrayAdapter<>(getActivity(),
                android.R.layout.simple_list_item_1, android.R.id.text1, MainActivity.PLAYLIST_ITEMS);
        setListAdapter(mAdapter);
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        View view = inflater.inflate(android.R.layout.list_content,
                container, false);
        view.setBackgroundColor(Color.WHITE);

        return view;
    }

    @Override
    public void onListItemClick(ListView l, View v, int position, long id) {
        super.onListItemClick(l, v, position, id);
    }
}
