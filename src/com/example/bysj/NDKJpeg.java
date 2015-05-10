package com.example.bysj;

import android.graphics.Bitmap;

						
public class NDKJpeg {

	static {
		System.loadLibrary("jpeg-ndk");
	}

	public static native String stringFromJNI();
	
	//public static native boolean setData(byte array[], int picSize);

	public static native void getBitmap(Bitmap picContext, byte[] buff, int imgSize);
}
