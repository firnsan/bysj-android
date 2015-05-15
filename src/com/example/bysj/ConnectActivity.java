package com.example.bysj;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;

public class ConnectActivity extends Activity {
	
	Button connBtn_;
	EditText addrTxt_;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		
		setContentView(R.layout.activity_connect);
		
		connBtn_ = (Button)findViewById(R.id.connect);
		addrTxt_ = (EditText)findViewById(R.id.address);
		
		connBtn_.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				String address = addrTxt_.getText().toString();
				Log.i("ConnActi", "address: "+address);
				
				Intent intent = getIntent();
				intent.putExtra("address", address);
				ConnectActivity.this.setResult(0, intent);
				ConnectActivity.this.finish();
			}
		});
	}
}