package com.example.bysj;



import android.app.Activity;
import android.app.Service;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Matrix;
import android.media.AudioManager;
import android.media.MediaPlayer;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.Vibrator;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnTouchListener;
import android.widget.Button;

public class MainActivity extends Activity implements OnClickListener, OnTouchListener, SurfaceHolder.Callback {

	MediaPlayer player_;
	SurfaceView surface_;
	SurfaceHolder holder_;
	
	Vibrator vibrator_; //震动
	
	
	NetThd netThread_;
	DrawPicThd drawThread_; //在surface上画图的线程
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		
		Log.i("test", "onCreate");
		
		surface_ = (SurfaceView)findViewById(R.id.surface);
		holder_ = surface_.getHolder();
		holder_.addCallback(this);
		//holder_.setFixedSize(320, 220); ////显示的分辨率,不设置为视频默认.很奇怪，设置了这个后，缩放的比例就不对了
		holder_.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);
		
		player_ = new MediaPlayer();
		vibrator_ = ( Vibrator ) getApplication().getSystemService(Service.VIBRATOR_SERVICE); 
		
		Button radioBtn = (Button)findViewById(R.id.radio);
		Button forwardBtn = (Button)findViewById(R.id.forward);
		Button backBtn = (Button)findViewById(R.id.back);
		Button leftBtn = (Button)findViewById(R.id.left);
		Button rightBtn = (Button)findViewById(R.id.right);
		
		radioBtn.setOnClickListener(this);
		forwardBtn.setOnTouchListener(this);
		backBtn.setOnTouchListener(this);
		leftBtn.setOnTouchListener(this);
		rightBtn.setOnTouchListener(this);
		
		Intent intent = new Intent(MainActivity.this, ConnectActivity.class);
		MainActivity.this.startActivityForResult(intent, 0);

	}
	
	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		Log.i("test", "keydown");
		return super.onKeyDown(keyCode, event);
	}

	@Override
	public void surfaceCreated(SurfaceHolder holder) {
		// TODO Auto-generated method stub
		/*
		Canvas canvas = holder_.lockCanvas();
		Bitmap back = BitmapFactory.decodeResource(getResources(), R.drawable.lena);
		Matrix matrix = new Matrix();
		
		float xScale = (float)surface_.getWidth() / back.getWidth();
		float yScale = (float)surface_.getHeight() / back.getHeight(); 
		matrix.setScale(xScale, yScale); 
		canvas.drawBitmap(back, matrix, null);
		holder.unlockCanvasAndPost(canvas);
		*/
	}

	@Override
	public void surfaceChanged(SurfaceHolder holder, int format, int width,
			int height) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void surfaceDestroyed(SurfaceHolder holder) {
		// TODO Auto-generated method stub
        super.onDestroy();
        if(player_.isPlaying()){
        	player_.stop();
        }
        player_.release();
        //Activity销毁时停止播放，释放资源。不做这个操作，即使退出还是能听到视频播放的声音	
	}

	@Override
	public void onClick(View v) {
		// TODO Auto-generated method stub
		switch(v.getId()) {
		case R.id.radio:
			Log.i("test", "radio被点击");
			//play();
			byte[] buf = {0x7F, 0x06, 0x01};
			Message msg = new Message();
			msg.what = 0x123;
			msg.obj = buf;
			netThread_.revHandler_.sendMessage(msg);
			break;

		}
		
	}
	
	private void play() {
		try{
			player_.setAudioStreamType(AudioManager.STREAM_MUSIC);
			player_.setDisplay(surface_.getHolder());
			player_.setDataSource("/storage/emulated/0/test.mp4");
			player_.prepare(); //缓冲
			player_.start(); //播放
		} catch(Exception e) {
			e.printStackTrace();
		}
	}

	
	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent intent) {
		// TODO Auto-generated method stub
		Bundle data = intent.getExtras();
		String address = data.getString("address");
		//新建ClientThread来连接小车、将按钮的操作发送到小车
		drawThread_ = new DrawPicThd(surface_);
		drawThread_.start();
		netThread_ = new NetThd(address, drawThread_.handler_);
		new Thread(netThread_).start();
		
	}
	
	
	@Override
	public boolean onTouch(View v, MotionEvent event) {
		// TODO Auto-generated method stub
		int action = event.getAction();
		byte[] buf = {0x7F,0x03,0x00};
		
		if (action == MotionEvent.ACTION_DOWN) {
			vibrator_.vibrate( new long[]{0,50},-1); 
		}
		
		switch(v.getId()) {
		case R.id.forward:
			buf[1] = 0x01;
			if (action == MotionEvent.ACTION_UP) {
				buf[2] = 0x00;
			}
			if (action == MotionEvent.ACTION_DOWN) {
				Log.i("test", "forward被down");  
				buf[2] = 0x01;
			}
			//Log.i("test", "forward被点击");  
			//buf[2] = 0x01;
			break;
		
		case R.id.back:
			buf[1] = 0x02;
			if (action == MotionEvent.ACTION_UP) {
				buf[2] = 0x00;
			}
			if (action == MotionEvent.ACTION_DOWN) {
				Log.i("test", "back被down");  
				buf[2] = 0x01;
			}

			//Log.i("test", "bakc被点击");  
			//buf[2] = 0x01;
			break;	
		
		case R.id.left:
			buf[1] = 0x03;
			if (action == MotionEvent.ACTION_UP) {
				buf[2] = 0x00;
			}
			if (action == MotionEvent.ACTION_DOWN) {
				Log.i("test", "left被down");  
				buf[2] = 0x01;
			}
			//Log.i("test", "left被点击");  
			//buf[2] = 0x01;
			break;
			
		case R.id.right:
			buf[1] = 0x04;
			if (action == MotionEvent.ACTION_UP) {
				buf[2] = 0x00;
			}
			if (action == MotionEvent.ACTION_DOWN) {
				Log.i("test", "right被down");  
				buf[2] = 0x01;
			}
			//Log.i("test", "right被点击");  
			//buf[2] = 0x01;
			break;
		}
		
		if (action == MotionEvent.ACTION_UP || action == MotionEvent.ACTION_DOWN){
			Message msg = new Message();
			msg.what = 0x123;
			msg.obj = buf;
			netThread_.revHandler_.sendMessage(msg);
		}

		return false;
	}
	
	@Override
	protected void onDestroy() {
		// TODO Auto-generated method stub
		netThread_.closeThd();
		super.onDestroy();
	}
}
