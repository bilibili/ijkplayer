package android.os;

import java.util.ArrayList;

@SimpleCClassName
public class Bundle {
    public Bundle();

    public int  getInt(String key, int defaultValue);
    public void putInt(String key, int value);

    public String getString(String key);
    public void   putString(String key, String value);

    public void putParcelableArrayList(String key, ArrayList value);
    public long getLong(String key);
    public void putLong(String key, long value);
}
