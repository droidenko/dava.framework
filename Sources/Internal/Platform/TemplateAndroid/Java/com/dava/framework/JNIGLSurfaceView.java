package com.dava.framework;

import java.util.ArrayList;

import com.bda.controller.ControllerListener;
import com.bda.controller.StateEvent;

import android.content.Context;
import android.graphics.PixelFormat;
import android.opengl.GLSurfaceView;
import android.os.Handler;
import android.util.AttributeSet;
import android.view.InputDevice;
import android.view.KeyEvent;
import android.view.MotionEvent;

public class JNIGLSurfaceView extends GLSurfaceView
{
	private JNIRenderer mRenderer = null;

	private native void nativeOnInput(int action, int id, float x, float y, double time, int source);
	private native void nativeOnKeyDown(int keyCode);
	
	public static int MSG_GL_INITIALIZED = 0x1;

	MOGAListener mogaListener = null;
	Handler messageHandler = new Handler()
	{
		@Override
		public void handleMessage(android.os.Message msg)
		{
			if (msg.what == MSG_GL_INITIALIZED)
			{
				setRenderMode(RENDERMODE_CONTINUOUSLY);
			}
		};
	};

	public JNIGLSurfaceView(Context context) 
	{
		super(context);
		Init();
	}

	public JNIGLSurfaceView(Context context, AttributeSet attrs)
	{
		super(context, attrs);
		Init();
	}

	private void Init()
	{
		this.getHolder().setFormat(PixelFormat.TRANSLUCENT);

		//setPreserveEGLContextOnPause(true);
		setEGLContextFactory(new JNIContextFactory());
		setEGLConfigChooser(new JNIConfigChooser(8, 8, 8, 8, 16, 8));

		mRenderer = new JNIRenderer(messageHandler);
		setRenderer(mRenderer);
		setRenderMode(RENDERMODE_WHEN_DIRTY);
		
		mogaListener = new MOGAListener(this);
	}
	
	@Override
	public void onPause()
	{
		super.onPause();
		setRenderMode(RENDERMODE_WHEN_DIRTY);
		queueEvent(new Runnable() 
		{
			public void run() 
			{
				try {
					Thread.sleep(500);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
				mRenderer.OnPause();
			}
		});
	}

	class InputRunnable implements Runnable
	{
		class InputEvent
		{
			int id;
			float x;
			float y;
			int source;
			
			InputEvent(int id, float x, float y, int source)
			{
				this.id = id;
				this.x = x;
				this.y = y;
				this.source = source;
			}
		}

		ArrayList<InputEvent> events;
		double time;
		int action;

		public InputRunnable(final android.view.MotionEvent event)
		{
			events = new ArrayList<InputEvent>();
			action = event.getActionMasked();
			if(action == MotionEvent.ACTION_MOVE)
			{
				int pointerCount = event.getPointerCount();
				for (int i = 0; i < pointerCount; ++i)
				{
					if((event.getSource() & InputDevice.SOURCE_CLASS_POINTER) > 0)
					{
						events.add(new InputEvent(event.getPointerId(i), event.getX(i), event.getY(i), event.getSource()));
					}
					if((event.getSource() & InputDevice.SOURCE_CLASS_JOYSTICK) > 0)
					{
						//InputEvent::id corresponds to axis id from UIEvent::eJoystickAxisID
						events.add(new InputEvent(0, event.getAxisValue(MotionEvent.AXIS_X, i), 0, event.getSource()));
						events.add(new InputEvent(1, event.getAxisValue(MotionEvent.AXIS_Y, i), 0, event.getSource()));
						events.add(new InputEvent(2, event.getAxisValue(MotionEvent.AXIS_Z, i), 0, event.getSource()));
						events.add(new InputEvent(3, event.getAxisValue(MotionEvent.AXIS_RX, i), 0, event.getSource()));
						events.add(new InputEvent(4, event.getAxisValue(MotionEvent.AXIS_RY, i), 0, event.getSource()));
						events.add(new InputEvent(5, event.getAxisValue(MotionEvent.AXIS_RZ, i), 0, event.getSource()));
						events.add(new InputEvent(6, event.getAxisValue(MotionEvent.AXIS_LTRIGGER, i), 0, event.getSource()));
						events.add(new InputEvent(7, event.getAxisValue(MotionEvent.AXIS_RTRIGGER, i), 0, event.getSource()));
						events.add(new InputEvent(8, event.getAxisValue(MotionEvent.AXIS_HAT_X, i), 0, event.getSource()));
						events.add(new InputEvent(9, event.getAxisValue(MotionEvent.AXIS_HAT_Y, i), 0, event.getSource()));
	    			}
	    		}
    		}
    		else
    		{
    			int actionIdx = event.getActionIndex();
    			assert(actionIdx <= event.getPointerCount());
    			events.add(new InputEvent(event.getPointerId(actionIdx), event.getX(actionIdx), event.getY(actionIdx), event.getSource()));
    		}
    	}
    	public InputRunnable(final com.bda.controller.MotionEvent event)
    	{
    		action = MotionEvent.ACTION_MOVE;
    		events = new ArrayList<InputEvent>();
        	int pointerCount = event.getPointerCount();
	    	for (int i = 0; i < pointerCount; ++i)
	    	{
	    		//InputEvent::id corresponds to axis id from UIEvent::eJoystickAxisID
	        	events.add(new InputEvent(0, event.getAxisValue(com.bda.controller.MotionEvent.AXIS_X, i), 0, InputDevice.SOURCE_CLASS_JOYSTICK));
	        	events.add(new InputEvent(1, event.getAxisValue(com.bda.controller.MotionEvent.AXIS_Y, i), 0, InputDevice.SOURCE_CLASS_JOYSTICK));
	        	events.add(new InputEvent(2, event.getAxisValue(com.bda.controller.MotionEvent.AXIS_Z, i), 0, InputDevice.SOURCE_CLASS_JOYSTICK));
	        	events.add(new InputEvent(5, event.getAxisValue(com.bda.controller.MotionEvent.AXIS_RZ, i), 0, InputDevice.SOURCE_CLASS_JOYSTICK));
	        	events.add(new InputEvent(6, event.getAxisValue(com.bda.controller.MotionEvent.AXIS_LTRIGGER, i), 0, InputDevice.SOURCE_CLASS_JOYSTICK));
	        	events.add(new InputEvent(7, event.getAxisValue(com.bda.controller.MotionEvent.AXIS_RTRIGGER, i), 0, InputDevice.SOURCE_CLASS_JOYSTICK));
    		}
    	}

		@Override
		public void run()
		{
			for (int i = 0; i < events.size(); ++i)
			{
				nativeOnInput(action, events.get(i).id, events.get(i).x, events.get(i).y, time, events.get(i).source);
			}
		}
    }

    class KeyInputRunnable implements Runnable
    {
    	int keyCode;
    	public KeyInputRunnable(int keyCode)
    	{
    		this.keyCode = keyCode;
    	}
    	
    	@Override
    	public void run() {
    		nativeOnKeyDown(keyCode);
    	}
    }
    
    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
    	queueEvent(new KeyInputRunnable(keyCode));
    	if(keyCode == KeyEvent.KEYCODE_BACK)
    		return super.onKeyDown(keyCode, event);
    	else
    		return true;
    }
    
    @Override
    public boolean onTouchEvent(MotionEvent event) 
    {
    	queueEvent(new InputRunnable(event));
        return true;
    }
    
    @Override
    public boolean onGenericMotionEvent(MotionEvent event)
    {
    	queueEvent(new InputRunnable(event));
    	return true;
    }
    
    class MOGAListener implements ControllerListener
    {
    	GLSurfaceView parent = null;
    	
    	MOGAListener(GLSurfaceView parent)
    	{
    		this.parent = parent;
    	}
    	
		@Override
		public void onKeyEvent(com.bda.controller.KeyEvent event)
		{
			if(event.getAction() == com.bda.controller.KeyEvent.ACTION_DOWN)
				parent.queueEvent(new KeyInputRunnable(event.getKeyCode()));
		}
		@Override
		public void onMotionEvent(com.bda.controller.MotionEvent event)
		{
			parent.queueEvent(new InputRunnable(event));
		}
		@Override
		public void onStateEvent(StateEvent event)
		{
			
		}
    }
}
