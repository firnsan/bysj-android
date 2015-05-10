package com.example.bysj;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.net.SocketTimeoutException;

import android.graphics.Bitmap;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.Log;

public class NetThd implements Runnable {
	private Socket socket_;
	
	Handler revHandler_; //从UI获取命令
	Handler drawHandler_; //属于画图线程的handler
	
	
	OutputStream os_ = null;
	InputStream is_ = null;
	
	public NetThd(Handler drawHandler) {
		drawHandler_ = drawHandler;
	}
	
	@Override
	public void run() {
		// TODO Auto-generated method stub
		try {
			socket_ = new Socket("192.168.137.1", 9898);
			os_ = socket_.getOutputStream();
			is_ = socket_.getInputStream();
		
			new Thread() {
				private byte[] buff = new byte[1000*1000]; 
				private Bitmap picContext = Bitmap.createBitmap(1366, 768, Bitmap.Config.ARGB_8888);;
				
				@Override
				public void run() {
					int bytesRead = 0; 
					int nBytes;
					try {
						while (true) {
							/* FLAG : 0xff */
							/* SIZE : 4 bytes */
							/* DATA : SIZE bytes */
							byte[] flag = new byte[1];
							byte[] binSize = new byte[4];
							is_.read(flag);
							is_.read(binSize);
							
							//System.arraycopy(buff, 1, binSize, 0, 4);
							int imgSize = byteArray2int(binSize);
							Log.i("NetThd", "imgSize: "+imgSize);

							while (bytesRead < imgSize) {
								nBytes = is_.read(buff, bytesRead, imgSize-bytesRead);
								bytesRead += nBytes;
							}

							bytesRead = 0;
							Log.i("NetThd", "imgSize received done! ");
							
							NDKJpeg.getBitmap(picContext, buff, imgSize);
							Message msg = new Message();
							msg.what = 0x123;
							msg.obj = picContext;
							drawHandler_.sendMessage(msg);
							
						}
					}
					catch (IOException e) {
						e.printStackTrace();
					}
				}
			}.start();
			Looper.prepare();
			
			revHandler_ = new Handler() {
				@Override
				public void handleMessage(Message msg) {
					try {
						os_.write((byte[])msg.obj);
					}
					catch (Exception e) {
						e.printStackTrace();
					}
				}
			};
			
			//启动loop
			Looper.loop();
		}
		catch (SocketTimeoutException e) {
			Log.i("test", "网络连接超时");
		}
		catch (Exception e) {
			e.printStackTrace();
		}
	}
	
	
	public int byteArray2int(byte[] b) {
		int v0 = (b[0] & 0xff);
		int v1 = (b[1] & 0xff) << 8;
		int v2 = (b[2] & 0xff) << 16;
		int v3 = (b[3] & 0xff) << 24;
		return v0 + v1 + v2 + v3;
	}
	
}