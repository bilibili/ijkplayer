package tv.danmaku.ijk.media.widget;

public class PlayerErrorInfo {
	private String ac;
	private String m;
	private String rip;
	private String br;
	private String errcode;
	private String token;

	public void setAc(String acString)
	{
		this.ac = acString;
	}
	
	public String getAc()
	{
		return this.ac;
	}
	
	public void setM(String mString)
	{
		this.m = mString;
	}
	
	public String getM()
	{
		return this.m;
	}
	
	public void setRip(String ripString)
	{
		this.rip = ripString;
	}
	
	public String getRip()
	{
		return this.rip;
	}
	
	public void setBr(String brString)
	{
		this.br = brString;
	}
	
	public String getBr()
	{
		return this.br;
	}
	
	public void setErrcode(String errCodeString)
	{
		this.errcode = errCodeString;
	}
	
	public String getErrcode()
	{
		return this.errcode;
	}
	
	public void setToken(String tokenString)
	{
		this.token = tokenString;
	}
	
	public String getToken()
	{
		return this.token;
	}
	
}
