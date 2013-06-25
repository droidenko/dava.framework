package com.dava.framework;

public class JNITextField {
	final static String TAG = "JNITextField";
	
	public static void ShowField(final float x, final float y, final float dx, final float dy, final String defaultText)
	{
		final JNIActivity activity = JNIActivity.GetActivity();
		final Object sync = new Object();
		synchronized (sync) {
			activity.runOnUiThread(new Runnable() {
				@Override
				public void run() {
					synchronized (sync) {
						activity.ShowEditText(x, y, dx, dy, defaultText);
						sync.notify();
					}
				}
			});
			try {
				sync.wait();
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}
	}
	
	public static void HideField()
	{
		final Object sync = new Object();
		synchronized (sync) {
			final JNIActivity activity = JNIActivity.GetActivity();
			String text = activity.GetEditText();
			activity.runOnUiThread(new Runnable() {
				@Override
				public void run() {
					synchronized (sync) {
						activity.HideEditText();
						sync.notify();
					}
				}
			});

			try {
				sync.wait();
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
			
			FieldHiddenWithText(text);
		};
	}
	
	public static native void FieldHiddenWithText(String text);
	public static native void TextFieldShouldReturn();
	public static native boolean TextFieldKeyPressed(int replacementLocation, int replacementLength, String replacementString);
}
