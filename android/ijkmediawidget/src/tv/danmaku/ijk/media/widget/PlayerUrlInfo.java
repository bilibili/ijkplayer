package tv.danmaku.ijk.media.widget;

public class PlayerUrlInfo {
	private String cdn;
	private String link;
	
	public PlayerUrlInfo() {

	}

	public PlayerUrlInfo(String cdn,String link) {
		this.cdn = cdn;
		this.link = link;
	}
	
	public void setCdn(String cdn)
	{
		this.cdn = cdn;
	}
	
	public String getCdn() {
		return this.cdn;
	}
	
	public void setLink(String link) {
		this.link = link;
	}
	
	public String getLink() {
		return this.link;
	}
}
