package tv.danmaku.ijk.media.sample.fragments;


import android.os.Bundle;
import android.app.Fragment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import tv.danmaku.ijk.media.sample.R;

/**
 * A simple {@link Fragment} subclass.
 */
public class FileExplorerFragment extends Fragment {


    public FileExplorerFragment() {
        // Required empty public constructor
    }


    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        return inflater.inflate(R.layout.fragment_file_explorer, container, false);
    }


}
