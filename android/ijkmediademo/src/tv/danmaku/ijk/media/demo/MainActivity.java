package tv.danmaku.ijk.media.demo;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;

public class MainActivity extends Activity {
	private String rtmpAddress;

	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.main);
		
        Button btn = (Button) this.findViewById(R.id.StartPlay1);
        final EditText et = (EditText) this.findViewById(R.id.RtmpAddress1);

//        et.setText("http://live.3gv.ifeng.com/zixun.m3u8");
//        et.setText("rtmp://wspub.live.hupucdn.com/prod/slk");
//        et.setText("http://v.iask.com/v_play_ipad.php?vid=99264895");
//        et.setText("rtmp://wsvideopull.smartcourt.cn/prod/sh_loft_b01ll");
//        et.setText("/sdcard/test.mp4");
        et.setText("vod://kanqiu/test_999?ac=pull&m=720p&st=1435904714&td=600000&u=12345678&ve=ws&k=368a643499ff07a8c4026796680be659ac67e1614cb8794bd77f2239fc668c04");

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
