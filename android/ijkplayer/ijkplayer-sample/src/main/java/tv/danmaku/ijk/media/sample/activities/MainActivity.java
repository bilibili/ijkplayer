package tv.danmaku.ijk.media.sample.activities;

import android.Manifest;
import android.content.Context;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.support.v4.app.ActivityCompat;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentTransaction;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.support.v7.widget.Toolbar;
import android.util.AttributeSet;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.List;

import tv.danmaku.ijk.media.sample.R;
import tv.danmaku.ijk.media.sample.content.MediaBean;
import tv.danmaku.ijk.media.sample.fragments.FileListFragment;
import tv.danmaku.ijk.media.sample.fragments.PlaylistFragment;

public class MainActivity extends AppCompatActivity {
    private static final String TAG = MainActivity.class.getSimpleName();

    private static final int MY_PERMISSIONS_REQUEST_READ_EXTERNAL_STORAGE = 1;
    private static final String SP_PLAYLIST = "sp_playlist";
    public static List<MediaBean> PLAYLIST_ITEMS = new ArrayList<>();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_app);

        if (ContextCompat.checkSelfPermission(this,
                Manifest.permission.READ_EXTERNAL_STORAGE)
                != PackageManager.PERMISSION_GRANTED) {
            if (ActivityCompat.shouldShowRequestPermissionRationale(this,
                    Manifest.permission.READ_EXTERNAL_STORAGE)) {
                // TODO: show explanation
            } else {
                ActivityCompat.requestPermissions(this,
                        new String[]{Manifest.permission.READ_EXTERNAL_STORAGE},
                        MY_PERMISSIONS_REQUEST_READ_EXTERNAL_STORAGE);
            }
        }

        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);

        SharedPreferences sp = getPreferences(MODE_PRIVATE);
        String playlistJson = sp.getString(SP_PLAYLIST, null);
        if (playlistJson != null) {
            try {
                JSONArray array = new JSONArray(playlistJson);
                for (int i = 0; i < array.length(); i++) {
                    JSONObject object = array.getJSONObject(i);
                    MediaBean bean = new MediaBean();
                    bean.fileName = object.getString(MediaBean.KEY_FILE_NAME);
                    bean.path = object.getString(MediaBean.KEY_PATH);
                    PLAYLIST_ITEMS.add(bean);
                }
            } catch (JSONException e) {
                Log.e(TAG, "parse json error");
                e.printStackTrace();
            }
        }

        loadPlaylistFragment();
    }

    @Override
    public View onCreateView(String name, Context context, AttributeSet attrs) {
        View view = super.onCreateView(name, context, attrs);
        return view;
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.menu_app, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        int id = item.getItemId();
        if (id == R.id.action_settings) {
            SettingsActivity.intentTo(this);
            return true;
        } else if (id == R.id.action_playlist) {
            Fragment newFragment = PlaylistFragment.newInstance();
            FragmentTransaction transaction = getSupportFragmentManager().beginTransaction();
            transaction.replace(R.id.body, newFragment).commit();
        } else if (id == R.id.action_explorer) {
            String initPath = "/sdcard";
            Fragment newFragment = FileListFragment.newInstance(initPath);
            FragmentTransaction transaction = getSupportFragmentManager().beginTransaction();
            transaction.replace(R.id.body, newFragment).commit();
        }

        return super.onOptionsItemSelected(item);
    }

    private void loadPlaylistFragment() {
        Fragment newFragment = PlaylistFragment.newInstance();
        FragmentTransaction transaction = getSupportFragmentManager().beginTransaction();
        transaction.replace(R.id.body, newFragment).commit();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        JSONArray array = new JSONArray();
        try {
            for (MediaBean bean : PLAYLIST_ITEMS) {
                JSONObject object = new JSONObject();
                object.put(MediaBean.KEY_FILE_NAME, bean.fileName);
                object.put(MediaBean.KEY_PATH, bean.path);
                array.put(object);
            }
        } catch (JSONException e) {
            e.printStackTrace();
        }
        // save to SharedPreferences
        SharedPreferences sp = getPreferences(MODE_PRIVATE);
        sp.edit().putString(SP_PLAYLIST, array.toString());
    }
}
