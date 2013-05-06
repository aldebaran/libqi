package com.archetype;

import com.aldebaran.R;
import com.aldebaran.R.id;
import com.aldebaran.R.layout;
import com.aldebaran.qimessaging.CallError;
import com.aldebaran.qimessaging.Application;
import com.aldebaran.qimessaging.GenericObject;
import com.aldebaran.qimessaging.Session;
import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.EditText;
import android.widget.Toast;

public class HelloAndroidActivity extends Activity {

  private static String TAG = "RobotTalk";
  private Session _session = null;
  @SuppressWarnings("unused")
  private Application _app = null;

  /**
   * Called when the activity is first created.
   * @param savedInstanceState If the activity is being re-initialized after
   * previously being shut down then this Bundle contains the data it most
   * recently supplied in onSaveInstanceState(Bundle). <b>Note: Otherwise it is null.</b>
   */
  @Override
  public void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    Log.i(TAG, "onCreate");
    setContentView(R.layout.main);

    _app = new Application(null);
  }
