package tv.danmaku.ijk.media.demo;

import com.mato.sdk.proxy.Proxy;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.os.Environment;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;

public class MainActivity extends Activity {
	private String rtmpAddress;

	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
		Proxy.start(this);
		
		setContentView(R.layout.main);
		
        Button btn = (Button) this.findViewById(R.id.StartPlay1);
        final EditText et = (EditText) this.findViewById(R.id.RtmpAddress1);

//        et.setText("http://live.3gv.ifeng.com/zixun.m3u8");
//        et.setText("rtmp://wspub.live.hupucdn.com/prod/slk");
//        et.setText("rtmp://wsvideopush.smartcourt.cn/prod/tmtest");
        et.setText("http://v.iask.com/v_play_ipad.php?vid=99264895");
//        et.setText("rtmp://wsvideopull.smartcourt.cn/prod/sh_loft_b01ll");
//        et.setText(Environment.getExternalStorageDirectory() + "/xmc1445234526439.mp4");
//        et.setText("vod://kanqiu/test_shiyi?ac=pull&m=720p&st=1436859023&td=60000&u=12345678&ve=ws&k=0f062b3824a5e0f787e1e35ff81f17a2ba025ff7f693c69460ec989b5f68eef8");

        btn.setOnClickListener(new OnClickListener(){

        	public void onClick(View v) {
				
        		rtmpAddress = et.getText().toString();
        		
        		Intent intent = new Intent(MainActivity.this, VideoPlayerActivity.class);
        		intent.putExtra("videoPath", rtmpAddress);
        		startActivity(intent);
			}
        });
	}
}
