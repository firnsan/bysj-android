package com.example.bysj;

import java.io.InputStream;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Matrix;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.view.SurfaceView;

class DrawPicThd extends Thread {
	private SurfaceView sv_;
	Handler handler_; //处理来自别的线程关于画图的消息
	
	
	public DrawPicThd(SurfaceView sv) {
		sv_ = sv;
		handler_ = new Handler() {
			@Override
			public void handleMessage(Message msg) {
				Bitmap picContext = (Bitmap)msg.obj;
				Canvas canvas = sv_.getHolder().lockCanvas();
				
				Matrix matrix = new Matrix();
				float xScale = (float)sv_.getWidth() / picContext.getWidth();
				float yScale = (float)sv_.getHeight() / picContext.getHeight(); 
				
				matrix.setScale(xScale, yScale); 
				canvas.drawBitmap(picContext, matrix, null);
				sv_.getHolder().unlockCanvasAndPost(canvas);
				
			}
		};
	}
	
	public void run() {
		Looper.prepare();
		Looper.loop();
	}
	

}