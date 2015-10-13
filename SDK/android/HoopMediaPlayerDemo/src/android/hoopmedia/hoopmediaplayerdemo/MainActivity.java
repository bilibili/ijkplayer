package android.hoopmedia.hoopmediaplayerdemo;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;

public class MainActivity extends Activity {
	private String rtmpAddress;
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.main);
		
        Button btn = (Button) this.findViewById(R.id.StartPlay1);
        final EditText et = (EditText) this.findViewById(R.id.RtmpAddress1);

        et.setText("rtmp://live.hkstv.hk.lxdns.com/live/hks");
        

        btn.setOnClickListener(new OnClickListener(){

        	public void onClick(View v) {
				
        		rtmpAddress = et.getText().toString();
        		
        		Intent intent = new Intent(MainActivity.this, VideoPlayerActivity.class);
        		intent.putExtra("RtmpAddress", rtmpAddress);
        		startActivity(intent);
			}
        });
	}
}
