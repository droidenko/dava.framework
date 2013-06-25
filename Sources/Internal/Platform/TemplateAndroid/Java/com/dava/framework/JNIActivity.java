package com.dava.framework;

import android.app.Activity;
import android.content.Context;
import android.graphics.Color;
import android.graphics.Rect;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.os.Handler;
import android.text.InputFilter;
import android.text.Spanned;
import android.util.Log;
import android.util.TypedValue;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.ViewGroup;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.EditText;
import android.widget.FrameLayout;
import org.fmod.FMODAudioDevice;

import com.bda.controller.Controller;

import android.widget.TextView;
import android.widget.TextView.OnEditorActionListener;

public abstract class JNIActivity extends Activity implements JNIAccelerometer.JNIAccelerometerListener
{
	private static int errorState = 0;

	private JNIAccelerometer accelerometer = null;
	private JNIGLSurfaceView glView = null;
	private EditText editText = null;
    
	private FMODAudioDevice fmodDevice = new FMODAudioDevice();
	
	private Controller mController;
	
    private native void nativeOnCreate(boolean isFirstRun);
    private native void nativeOnStart();
    private native void nativeOnStop();
    private native void nativeOnDestroy();
    private native void nativeIsFinishing();
    private native void nativeOnAccelerometer(float x, float y, float z);
    
    private boolean isFirstRun = true;
    
    public abstract JNIGLSurfaceView GetSurfaceView();
    
    private static JNIActivity activity = null;
    
    public static JNIActivity GetActivity()
	{
		return activity;
	}
    
    @Override
    public void onCreate(Bundle savedInstanceState) 
    {
    	activity = this;
        super.onCreate(savedInstanceState);
        
        if(null != savedInstanceState)
        {
        	isFirstRun = savedInstanceState.getBoolean("isFirstRun");
        }
        
    	// The activity is being created.
        Log.i(JNIConst.LOG_TAG, "[Activity::onCreate]");

        // initialize accelerometer
        SensorManager sensorManager = (SensorManager)getSystemService(Context.SENSOR_SERVICE);
        accelerometer = new JNIAccelerometer(this, sensorManager);

        // initialize GL VIEW
        glView = GetSurfaceView();
        assert(glView != null);
        glView.setFocusableInTouchMode(true);
        glView.setClickable(true);
        glView.setFocusable(true);
        glView.requestFocus();

        mController = Controller.getInstance(this);
        if(mController != null)
        {
        	mController.init();
        	mController.setListener(glView.mogaListener, new Handler());
        }

        Log.i(JNIConst.LOG_TAG, "[Activity::onCreate] isFirstRun is " + isFirstRun); 
        nativeOnCreate(isFirstRun);
    }
    
    @Override
    protected void onStart()
    {
    	super.onStart();
    	fmodDevice.start();
    	// The activity is about to become visible.
    	
        Log.i(JNIConst.LOG_TAG, "[Activity::onStart]");
        
        //editText.setVisibility(EditText.INVISIBLE);
        
        //call native method
        nativeOnStart();
    }
    
    @Override
    protected void onRestart()
    {
    	super.onRestart();
    	// The activity is about to become visible.
    	
        Log.i(JNIConst.LOG_TAG, "[Activity::onRestart]");
        
        //TODO: VK: may be usefull
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) 
    {
        Log.i(JNIConst.LOG_TAG, "[Activity::onSaveInstanceState]");

        outState.putBoolean("isFirstRun", isFirstRun);
    	
    	super.onSaveInstanceState(outState);
    }
    
    @Override
    protected void onResume() 
    {
        super.onResume();
        // The activity has become visible (it is now "resumed").
		Log.i(JNIConst.LOG_TAG, "[Activity::onResume] start");

		if(mController != null)
		{
			mController.onResume();
		}
		
        // activate accelerometer
        if(null != accelerometer)
        {
        	if(accelerometer.IsSupported())
        	{
        		accelerometer.Start();
        	}
        }
        
        // activate GLView 
        if(null != glView)
        {
        	glView.onResume();
        }
        
        if (editText != null)
        {
        	//YZ restore keyboard
    		InputMethodManager input = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
    		input.showSoftInput(editText, InputMethodManager.SHOW_FORCED);
        }
        
        Log.i(JNIConst.LOG_TAG, "[Activity::onResume] finish");
    }

    
    @Override
    protected void onPause() 
    {
        super.onPause();
        // Another activity is taking focus (this activity is about to be "paused").

        Log.i(JNIConst.LOG_TAG, "[Activity::onPause] start");

		if(mController != null)
		{
			mController.onPause();
		}
		
        // deactivate accelerometer
        if(null != accelerometer)
        {
        	if(accelerometer.IsActive())
        	{
        		accelerometer.Stop();
        	}
        }

        // deactivate GLView 
        if(null != glView)
        {
        	glView.onPause();
        }
        
        boolean isActivityFinishing = isFinishing();
        Log.i(JNIConst.LOG_TAG, "[Activity::onPause] isActivityFinishing is " + isActivityFinishing);
        if(isActivityFinishing)
        {
        	nativeIsFinishing();
        }
        
        if (editText != null)
        {
        	//YZ hide keyboard
        	InputMethodManager input = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
        	input.hideSoftInputFromWindow(editText.getWindowToken(), 0);
        }

        Log.i(JNIConst.LOG_TAG, "[Activity::onPause] finish");
    }

    @Override
    protected void onStop()
    {
        Log.i(JNIConst.LOG_TAG, "[Activity::onStop] start");
        
        fmodDevice.stop();
        
        //call native method
        nativeOnStop();

        Log.i(JNIConst.LOG_TAG, "[Activity::onStop] finish");
        
        super.onStop();
    	// The activity is no longer visible (it is now "stopped")
    }
    
    
    @Override
    protected void onDestroy()
    {
        Log.i(JNIConst.LOG_TAG, "[Activity::onDestroy] start");

        if(mController != null)
        	mController.exit();
        
        //call native method
        nativeOnDestroy();

        Log.i(JNIConst.LOG_TAG, "[Activity::onDestroy] finish");

        super.onDestroy();
    	// The activity is about to be destroyed.
    }

    @Override
    protected void onPostResume() 
    {
        Log.i(JNIConst.LOG_TAG, "[Activity::onPostResume] ****");
        
    	super.onPostResume();
    }
    
    public void onAccelerationChanged(float x, float y, float z)
	{
		nativeOnAccelerometer(x, y, z);
	}
    
    static boolean inputFilterRes = false;
    private void InitEditText(EditText editText, Rect rect)
    {
		FrameLayout.LayoutParams params = new FrameLayout.LayoutParams(rect.width(), rect.height());
		params.leftMargin = rect.left;
		params.topMargin = rect.top;
		params.gravity = Gravity.LEFT | Gravity.TOP;

		editText.setPadding(0, 0, 0, 0);
		
		editText.setSingleLine(true);
		int fontSize = (int)(rect.height() / 1.45f);
		editText.setTextSize(TypedValue.COMPLEX_UNIT_PX, fontSize);

		editText.setBackgroundColor(Color.BLACK);
		editText.setTextColor(Color.WHITE);

		editText.setImeOptions(EditorInfo.IME_FLAG_NO_EXTRACT_UI | EditorInfo.IME_ACTION_DONE);

		InputFilter inputFilter = new InputFilter() {
			
			@Override
			public CharSequence filter(final CharSequence source, final int start, final int end,
					Spanned dest, int dstart, int dend) {
				if (source.length() > 1)
					return source;
				
				inputFilterRes = false;
				final Object mutex = new Object();
				glView.queueEvent(new Runnable() {
					public void run() {
						inputFilterRes = JNITextField.TextFieldKeyPressed(start, end - start, source.toString());
						synchronized (mutex) {
							mutex.notify();
						}
					}
				});
				synchronized (mutex) {
					try {
						mutex.wait();
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
				}
				if (inputFilterRes)
					return source;
				return "";
			}
		};
        editText.setFilters(new InputFilter[]{inputFilter});

		editText.setOnEditorActionListener(new OnEditorActionListener() {
			
			@Override
			public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
				if (actionId == EditorInfo.IME_ACTION_DONE) {
					glView.queueEvent(new Runnable() {
						@Override
						public void run() {
							JNITextField.TextFieldShouldReturn();
						}
					});
					return true;
				}
				return false;
			}
		});
		addContentView(editText, params);
		
		editText.setInputType(EditorInfo.TYPE_CLASS_TEXT | EditorInfo.TYPE_TEXT_VARIATION_VISIBLE_PASSWORD);
    }
	
	public void ShowEditText(float x, float y, float dx, float dy, String defaultText)
	{
		if (editText != null) {
			RemoveEditText();
		}

		editText = new EditText(this);
		InitEditText(editText, new Rect((int)x, (int)y, (int)(x + dx), (int)(y + dy)));

		editText.setText(defaultText);
		editText.setSelection(editText.getText().length());

		editText.requestFocus();
		InputMethodManager input = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
		input.showSoftInput(editText, InputMethodManager.SHOW_FORCED);
	}
	
	public void HideEditText()
	{
		if (editText == null)
			return;

		InputMethodManager input = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
		input.hideSoftInputFromWindow(editText.getWindowToken(), 0);

		RemoveEditText();
	}

	private void RemoveEditText() {
		if (editText != null) {
			ViewGroup parent = (ViewGroup) editText.getParent();
			if (parent != null)
				parent.removeView(editText);
			editText = null;
		}
	}
	
	public String GetEditText()
	{
		if (editText != null)
			return editText.getText().toString();
		return "";
	}
}
