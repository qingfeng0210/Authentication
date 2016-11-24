package cn.dacas.authentication.util;

import android.content.Context;
import android.content.SharedPreferences;
import android.util.Log;

import org.json.JSONArray;

import java.util.ArrayList;

public class SharedPreferencesHelper {

	private String MSP_SETTING = "SECURECORESETTING";
	private static SharedPreferencesHelper sharedPreferencesHelper = null;
	
	public static SharedPreferencesHelper getInstance(Context context) {
		if (sharedPreferencesHelper == null) {
			synchronized (SharedPreferencesHelper.class) {
				if (sharedPreferencesHelper == null) {
					sharedPreferencesHelper = new SharedPreferencesHelper();
					sharedPreferencesHelper.setContext(context);
					return sharedPreferencesHelper;
				}
			}
		}
		return sharedPreferencesHelper;
	}

	private Context context;

	public void setContext(Context context) {
		this.context = context;
	}
	
	public boolean getBoolean(String key, boolean defValue) {
		try {
			return getSP().getBoolean(key, defValue);
		} catch (NullPointerException exception) {
			Log.d("hcj", ""+exception);
			return defValue;
		}
	}
	
	public void putBoolean(String key, boolean value) {
		try {
			SharedPreferences.Editor editor = getSP().edit();
			editor.putBoolean(key, value);
			editor.commit();
		} catch (NullPointerException exception) {
			Log.d("hcj", ""+exception);
		}
	}
	

	public long getLong(String key, long defValue) {
		try {
			return getSP().getLong(key, defValue);
		} catch (NullPointerException exception) {
			Log.d("hcj", ""+exception);
			return defValue;
		}
	}
	
	public void putLong(String key, long value) {
		try {
			SharedPreferences.Editor editor = getSP().edit();
			editor.putLong(key, value);
			editor.commit();
		} catch (NullPointerException exception) {
			Log.d("hcj", ""+exception);
		}
	}
	
	public int getInt(String key, int defaultValue) {
		try {
			return getSP().getInt(key, defaultValue);
		} catch (Exception e) {
			return defaultValue;

		}
	}
	
	public void putInt(String key, int value) {
		try {
			SharedPreferences.Editor editor = getSP().edit();
			editor.putInt(key, value);
			editor.commit();
		} catch (Exception e) {
		}
	}
	
	public String getString(String key, String defValue) {
		try {
			return getSP().getString(key, defValue);
		} catch (NullPointerException e) {
			return defValue;
		}
	}
	
	public void putString(String key, String value) {
		try {
			SharedPreferences.Editor editor = getSP().edit();
			editor.putString(key, value);
			editor.commit();
		} catch (NullPointerException e) {
		}
	}

	public ArrayList<String> getStringList(String key) {
		ArrayList<String> list=new ArrayList<String>();
		try {
			String raw=getSP().getString(key, null);
			if (raw!=null) {
				JSONArray array=new JSONArray(raw);
				for (int i=0;i<array.length();i++)
					list.add(array.getString(i));
			}
			return list;
		} catch (Exception e) {
			return list;
		}
	}

	public void putStringList(String key, ArrayList<String> list) {
		try {
			SharedPreferences.Editor editor = getSP().edit();
			JSONArray array=new JSONArray();
			for(String str:list)
				array.put(str);
			editor.putString(key,array.toString());
			editor.commit();
		} catch (NullPointerException e) {
			e.printStackTrace();
		}
	}

	public void clear(String key) {
		try {
			SharedPreferences.Editor editor = getSP().edit();
			editor.remove(key);
			editor.commit();
		} catch (NullPointerException e) {
		}
	}

	public void clear() {
		try {
			SharedPreferences.Editor editor = getSP().edit();
			editor.clear();	
			editor.commit();
		} catch (NullPointerException e) {
		}
	}
	
	private SharedPreferences getSP() {
		return context.getSharedPreferences(MSP_SETTING, Context.MODE_PRIVATE);
	}

}
