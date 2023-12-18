package com.defold.firebase.remoteconfig;

import androidx.annotation.NonNull;
import android.util.Log;

import com.google.android.gms.tasks.Task;
import com.google.android.gms.tasks.OnCompleteListener;
import com.google.firebase.remoteconfig.FirebaseRemoteConfig;
import com.google.firebase.remoteconfig.FirebaseRemoteConfigSettings;

import org.json.JSONObject;
import org.json.JSONException;

public class FirebaseRemoteConfigJNI {
	private static final String TAG = "FirebaseRemoteConfigJNI";

	public static native void firebaseAddToQueue(int msg, String json);

	private static final int MSG_ERROR          	= 0;
	private static final int MSG_INITIALIZED		= 1;
	private static final int MSG_DEFAULTS_SET       = 2;
	private static final int MSG_SETTINGS_UPDATED	= 3;
	private static final int MSG_FETCHED        	= 4;
	private static final int MSG_ACTIVATED      	= 5;

	private static final long kDefaultCacheExpirationInSeconds = 60 * 60 * 12;
	private static final long kDefaultTimeoutInSeconds = 30;


	// duplicate of enums from firebase_remoteconfig_callback.h:
  	// CONSTANTS:

  	private FirebaseRemoteConfig firebaseRemoteConfig;
  	private FirebaseRemoteConfigSettings.Builder configSettingsBuilder;

  	public FirebaseRemoteConfigJNI() {

	}

	public void initialize() {
		try {
	  		this.firebaseRemoteConfig = FirebaseRemoteConfig.getInstance();

        	this.configSettingsBuilder = new FirebaseRemoteConfigSettings.Builder();

            this.configSettingsBuilder.setMinimumFetchIntervalInSeconds(kDefaultCacheExpirationInSeconds);
            this.configSettingsBuilder.setFetchTimeoutInSeconds(kDefaultTimeoutInSeconds);

        	this.firebaseRemoteConfig.setConfigSettingsAsync(this.configSettingsBuilder.build()).addOnCompleteListener(new OnCompleteListener<Void>() {
	            @Override
	            public void onComplete(@NonNull Task<Void> task) {
	                if (task.isSuccessful()) {
						sendSimpleMessage(MSG_INITIALIZED);
	                } else {
						sendErrorMessage("Remote Config. Unable to initialize");
	                }
	            }
	        });
		} catch (Exception e) {
			sendErrorMessage(e.getLocalizedMessage());
		}
	}

	public void setDefaults() {

	}

	public void setMinimumFetchInterval(double fetchInterval) {
		try {
	        this.configSettingsBuilder.setMinimumFetchIntervalInSeconds((long)fetchInterval);
        	this.firebaseRemoteConfig.setConfigSettingsAsync(this.configSettingsBuilder.build()).addOnCompleteListener(new OnCompleteListener<Void>() {
	            @Override
	            public void onComplete(@NonNull Task<Void> task) {
	                if (task.isSuccessful()) {
						sendSimpleMessage(MSG_SETTINGS_UPDATED);
	                } else {
						sendErrorMessage("Remote Config. Unable to change settings");
	                }
	            }
	        });
		} catch (Exception e) {
			sendErrorMessage(e.getLocalizedMessage());
		}
	}

	public void setTimeout(double timeOut) {
		try {
	        this.configSettingsBuilder.setFetchTimeoutInSeconds((long)timeOut);
        	this.firebaseRemoteConfig.setConfigSettingsAsync(this.configSettingsBuilder.build()).addOnCompleteListener(new OnCompleteListener<Void>() {
	            @Override
	            public void onComplete(@NonNull Task<Void> task) {
	                if (task.isSuccessful()) {
						sendSimpleMessage(MSG_SETTINGS_UPDATED);
	                } else {
						sendErrorMessage("Remote Config. Unable to change settings");
	                }
	            }
	        });
		} catch (Exception e) {
			sendErrorMessage(e.getLocalizedMessage());
		}
	}
		
	public boolean getBoolean(String key) {
		return this.firebaseRemoteConfig.getBoolean(key);
	}

	public double getNumber(String key) {
		return this.firebaseRemoteConfig.getDouble(key);
	}

	public String getData(String key) {
		return this.firebaseRemoteConfig.getString(key);
	}

	public String getString(String key) {
		return this.firebaseRemoteConfig.getString(key);
	}

	public void fetch() {
        firebaseRemoteConfig.fetch().addOnCompleteListener(new OnCompleteListener<Void>() {
            @Override
            public void onComplete(@NonNull Task<Void> task) {
                if (task.isSuccessful()) {
					sendSimpleMessage(MSG_FETCHED);
                } else {
					sendErrorMessage("Remote Config. Unable to fetch");
                }
            }
        });
	}

	public void activate() {
        firebaseRemoteConfig.activate().addOnCompleteListener(new OnCompleteListener<Boolean>() {
            @Override
            public void onComplete(@NonNull Task<Boolean> task) {
                if (task.isSuccessful()) {
					sendSimpleMessage(MSG_ACTIVATED);
                } else {
					sendErrorMessage("Remote Config. Unable to activate");
                }
            }
        });
	}

	public void fetchAndActivate() {
        firebaseRemoteConfig.fetchAndActivate().addOnCompleteListener(new OnCompleteListener<Boolean>() {
            @Override
            public void onComplete(@NonNull Task<Boolean> task) {
                if (task.isSuccessful()) {
					sendSimpleMessage(MSG_FETCHED);
					sendSimpleMessage(MSG_ACTIVATED);
                } else {
					sendErrorMessage("Remote Config. Unable to fetch and activate");
                }
            }
        });
	}

	private String getJsonConversionErrorMessage(String errorText) {
		String message = null;
		
		try {
			JSONObject obj = new JSONObject();
			obj.put("error", errorText);
			message = obj.toString();
		} catch (JSONException e) {
			message = "{ \"error\": \"Error while converting simple message to JSON.\"}";
		}

		return message;
	}

	private void sendErrorMessage(String errorText) {
		String message = getJsonConversionErrorMessage(errorText);
		Log.d(TAG, "Remote Config Error");
		Log.d(TAG, message);
		firebaseAddToQueue(MSG_ERROR, message);
	}

	private void sendSimpleMessage(int msg) {
		firebaseAddToQueue(msg, "{}");
	}	
}