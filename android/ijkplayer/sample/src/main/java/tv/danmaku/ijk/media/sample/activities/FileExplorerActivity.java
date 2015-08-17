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

package tv.danmaku.ijk.media.sample.activities;

import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentTransaction;
import android.support.v7.app.ActionBarActivity;
import android.widget.TextView;

import java.io.File;

import tv.danmaku.ijk.media.sample.R;
import tv.danmaku.ijk.media.sample.VideoPlayerActivity;
import tv.danmaku.ijk.media.sample.fragments.FileListFragment;

public class FileExplorerActivity extends ActionBarActivity implements FileListFragment.OnClickFileListener {

    private TextView mPathView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_file_explorer);

        mPathView = (TextView) findViewById(R.id.path_view);

        doOpenDirectory("/", false);
    }

    private void doOpenDirectory(String path, boolean addToBackStack) {
        mPathView.setText(path);

        Fragment newFragment = FileListFragment.newInstance(path);
        FragmentTransaction transaction = getSupportFragmentManager().beginTransaction();

        transaction.replace(R.id.file_explorer_view, newFragment);

        if (addToBackStack)
            transaction.addToBackStack(null);
        transaction.commit();
    }

    @Override
    public void onClickFile(File f) {
        if (f.isDirectory()) {
            doOpenDirectory(f.toString(), true);
        } else {
            VideoPlayerActivity.intentTo(this, f.getPath(), f.getName());
        }
    }
}
