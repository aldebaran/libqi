package com.aldebaran;

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

public class HelloAndroidActivity extends Activity
{

  private static String TAG = "RobotTalk";

  @SuppressWarnings("unused")
  private        Application app = null;
  private        Session session = null;

  /**
   * Called when the activity is first created.
   * @param savedInstanceState If the activity is being re-initialized after
   * previously being shut down then this Bundle contains the data it most
   * recently supplied in onSaveInstanceState(Bundle). <b>Note: Otherwise it is null.</b>
   */
  @Override
  public void onCreate(Bundle savedInstanceState)
  {
    super.onCreate(savedInstanceState);
    Log.i(TAG, "onCreate");
    setContentView(R.layout.main);

    app = new Application(null);
  }

  public void connectSD(View view)
  {
    EditText mEdit     = (EditText)findViewById(R.id.editIP);
    EditText EditPort  = (EditText)findViewById(R.id.editPort);
    EditText editSentences = (EditText)findViewById(R.id.editText);
    Context context = getApplicationContext();

    String url = "tcp://" + mEdit.getText().toString() + ":" + EditPort.getText().toString();
    Log.v("com.aldebaran.RobotTalk.connectSD", "Connecting to " + url);

    _session = new Session();
    try
    {
      _session.connect(url);
    }
    catch (Exception e)
    {
      Toast toast = Toast.makeText(context, "Connection Error : " + e.getMessage(), Toast.LENGTH_SHORT);
      toast.show();
      Log.v("com.aldebaran.RobotTalk.connectSD", "Connection Error : " + e.getMessage());
      return;
    }

    Log.v("com.aldebaran.RobotTalk.connectSD", "Trying to get a proxy on serviceTest");
    GenericObject proxy = _session.service("ALTextToSpeech");

    if (proxy == null)
    {
      Toast toast = Toast.makeText(context, "Cannot get proxy on ALTextToSpeech", Toast.LENGTH_SHORT);
      toast.show();
      Log.v("com.aldebaran.RobotTalk.connectSD", "Failure ! Cannot get proxy on ALTextToSpeech");
      return;
    }

    try
    {
      Log.v("com.aldebaran.RobotTalk.connectSD", "Saying : " + editSentences.getText().toString());
      proxy.call("say", editSentences.getText().toString());
    } catch (CallError e) {
      Toast toast = Toast.makeText(context, "Error : " + e.getMessage(), Toast.LENGTH_SHORT);
      toast.show();
      return;
    } catch (Exception e) {
      Toast toast = Toast.makeText(context, "Error : " + e.getMessage(), Toast.LENGTH_SHORT);
      toast.show();
      return;
    }

    Log.v("com.aldebaran.RobotTalk.connectSD", "Done");
    Toast toast = Toast.makeText(context, "Done", Toast.LENGTH_SHORT);
    toast.show();
  }
}
