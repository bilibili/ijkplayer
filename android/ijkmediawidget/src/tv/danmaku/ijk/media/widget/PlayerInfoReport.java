package tv.danmaku.ijk.media.widget;

import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.util.Log;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.net.UnknownHostException;
import java.util.Collections;
import java.util.List;
import org.apache.http.conn.util.InetAddressUtils;

import com.google.gson.Gson;

public class PlayerInfoReport {
	private VideoView videoView = null;
	
//	private String ve = null; //CDN Name
//	private String m = null; //mode(720p,1080p)
//	private String rip = null; //CDN IP
//	private String lip = null; //Local IP
//	private String bl = null; //Current Cache Length
//	private String bc = null; //stalled count / one minute
//	private String bt = null; //all stalled count
//	private String br = null; //bitrate
//	private String token = null; //
//	private String oc = null; //stream open count
//	private String cd = null; // stalled time / one minute
	
	public PlayerInfoReport (VideoView videoView) {
		this.videoView = videoView;
	}
	
	
	private HandlerThread handlerThread=null;
	private Handler handler=null;
	private Runnable runnable=null;
	public void startReport()
	{
		if(handlerThread==null)
		{
			handlerThread = new HandlerThread("PlayerInfoReport");
			handlerThread.start();
			Looper looper = handlerThread.getLooper();
			handler = new Handler(looper);
			
			runnable = new Runnable() {
				
				@Override
				public void run() {
					String sendData = getPlayerInfoWithJsonFormat();
					Log.v("PlayerInfoReport", sendData);
					udpSend(sendData);
					
					handler.postDelayed(runnable, 60*1000);
				}
			};
			
			handler.postDelayed(runnable, 60*1000);
		}
	}
	
	public void endReport()
	{
		if (handler!=null) {
			handler.removeCallbacks(runnable);
			handler = null;
		}
		
		if (handlerThread!=null) {
			handlerThread.quit();
			handlerThread = null;
		}
	}
	
	private DatagramSocket udpSocket = null;
	private InetAddress serverAddress = null;

	private void udpSend(String data)
	{
		if (udpSocket==null) {
			try {
				udpSocket = new DatagramSocket();
			} catch (SocketException e) {
				Log.e("PlayerInfoReport", "UDP Report Fail");
				return;
			}
		}

		if (serverAddress==null) {
			try {
				serverAddress = InetAddress.getByName("192.168.9.117");
			} catch (UnknownHostException e) {
				Log.e("PlayerInfoReport", "UDP Report Fail");
				return;
			}
		}
		
		DatagramPacket udpPacket = new DatagramPacket(data.getBytes(), data.length(), serverAddress,
                33333);
		
		try {
			udpSocket.send(udpPacket);
		} catch (IOException e) {
			Log.e("PlayerInfoReport", "UDP Report Fail");
			return;
		}
		
		udpSocket.close();
		udpSocket = null;
	}

	private Gson gson = null;
	private PlayerInfo playerInfo = null;
	
	private String getPlayerInfoWithJsonFormat()
	{
		if (gson==null) {
			gson = new Gson();
		}
		
		if (playerInfo == null) {
			playerInfo = new PlayerInfo();
		}
		
//		playerInfo.setVe(this.getVe());
		playerInfo.setM(this.getM());
		playerInfo.setRip(this.getRip());
		playerInfo.setLip(this.getLip());
		playerInfo.setBl(this.getBl());
		playerInfo.setBc(this.getBc());
		playerInfo.setBt(this.getBt());
		playerInfo.setBr(this.getBr());
		playerInfo.setToken(this.getToken());
		playerInfo.setOc(this.getOc());
		playerInfo.setCd(this.getCd());

		return gson.toJson(playerInfo);
	}
	
	private String getVe() {
		if (videoView==null) {
			return "unknown";
		}
		
		String cdnName = videoView.getCdnName();
		if (cdnName==null) {
			return "unknown";
		}else {
			return cdnName;
		}
	}
	
	private String getM()
	{
		if (videoView==null) {
			return "unknown";
		}
		
		if (videoView.getVideoWidth() == 1920 && videoView.getVideoHeight()==1080) {
			return "1080p";
		}else if (videoView.getVideoWidth() == 1280 && videoView.getVideoHeight()==720) {
			return "720p";
		}else if (videoView.getVideoWidth() == 640 && videoView.getVideoHeight()==480) {
			return "480p";
		}else {
			return String.format("%dx%d", videoView.getVideoWidth(),videoView.getVideoHeight());
		}
	}
	
	private String getRip()
	{
		if (videoView==null) {
			return "unknown";
		}
		
		String ripString = videoView.getRemoteIpAddress();
		if (ripString==null) {
			return "unknown";
		}else {
			return ripString;
		}
	}
	
	private String getLocalIpAddress() {
    	try {
    		String ipv4;
    		List<NetworkInterface>  nilist = Collections.list(NetworkInterface.getNetworkInterfaces());
    		for (NetworkInterface ni: nilist) 
    		{
    			List<InetAddress>  ialist = Collections.list(ni.getInetAddresses());
    			for (InetAddress address: ialist){
    				if (!address.isLoopbackAddress() && InetAddressUtils.isIPv4Address(ipv4=address.getHostAddress())) 
    				{ 
    					return ipv4;
    				}
    			}
    		}
 
    	} catch (SocketException ex) {
    		Log.e("PlayerInfoReport", ex.toString());
    	}
    	return null;
    }
	
	private String getLip() {
		String lipString = getLocalIpAddress();
		if (lipString==null) {
			return "unknown";
		}else {
			return lipString;
		}
	}
	
	private String getBl() {
		if (videoView==null) {
			return "0";
		}
		return Integer.valueOf(videoView.getPlayableDuration()).toString();
	}
	
	private String getBc() {
		if (videoView==null) {
			return "0";
		}
		
		return Integer.valueOf(videoView.getBuffingCountPerMinute()).toString();
	}
	
	private String getBt()
	{
		if (videoView==null) {
			return "0";
		}
		
		return Integer.valueOf(videoView.getAllbuffingCount()).toString();
	}
	
	private String getBr()
	{
		if (videoView==null) {
			return "0";
		}
		
		return Integer.valueOf(videoView.getBitRate()).toString();
	}
	
	private String getToken() {
		if (videoView==null) {
			return "unknown";
		}
		
		String token = videoView.getToken();
		if (token==null) {
			return "unknown";
		}else {
			return token;
		}
	}
	
	private String getOc() {
		if (videoView==null) {
			return "unknown";
		}
		return Integer.valueOf(videoView.getStreamOpenCount()).toString();
	}
	
	private String getCd() {
		if (videoView==null) {
			return "unknown";
		}
		return Integer.valueOf(videoView.getBuffingTimePerMinute()).toString();
	}
}
