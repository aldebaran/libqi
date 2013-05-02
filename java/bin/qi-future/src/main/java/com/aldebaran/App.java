package com.aldebaran;

import java.util.concurrent.ExecutionException;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

import com.aldebaran.qimessaging.Application;
import com.aldebaran.qimessaging.CallError;
import com.aldebaran.qimessaging.Callback;
import com.aldebaran.qimessaging.Future;
import com.aldebaran.qimessaging.GenericObject;
import com.aldebaran.qimessaging.Session;

/**
 * Hello Future!
 *
 */
public class App
{
  public static void main( String[] args )
  {
    Application _app = new Application(args);
    String sdAddr = "tcp://127.0.0.1:5555";

    if (args.length >= 1)
      sdAddr = args[0];

    Session client = new Session();

    // Connect to Service Directory.
    try
    {
      client.connect(sdAddr);
    }
    catch (Exception e)
    {
      System.out.println("Cannot connect to Service Directory : " + e.getMessage());
      return;
    }

    // Get proxy on serviceTest.
    GenericObject proxy = null;
    try
    {
      proxy = client.service("serviceTest");
    } catch (Exception e)
    {
      System.out.println("Cannot get proxy on serviceTest : " + e.getMessage());
      return;
    }


    // Call a 2s long function.
    Future<String> fut = null;
    try
    {
      fut = proxy.asyncCall("longReply", "plaf");
      fut.addCallback(new Callback<Object>() {

        public void onComplete(Future<Object> arg0, Object[] args)
        {
          System.out.println("On Complete !");
          System.out.println("Args : " + args[0] + ", size " + args.length);
        }

        public void onFailure(Future<Object> arg0, Object[] args)
        {
          System.out.println("On Failure !");
          System.out.println("Args : " + args[0] + ", size " + args.length);
        }

        public void onSuccess(Future<Object> arg0, Object[] args) {
          String ret = null;

          try
          {
            ret = (String) arg0.get();
          } catch (Exception e) {e.printStackTrace();}

          try
          {
            System.out.println("On Success ! : " + ret);
            System.out.println("Args : " + args[0]);
            System.out.println("Args : " + args[1]);
            System.out.println("Args : " + args[2]);
            System.out.println("Done");
          } catch (Exception e)
          {
            System.out.println("Error : " + e.getMessage());
          }
        }

      }, "YEAH !", 2, 3);

    } catch (CallError e)
    {
      System.out.println("Error calling answer function : " + e.getMessage());
      return;
    }

    // Wait for call to finish.
    while (fut.isDone() == false)
    {
      System.out.println("Call not finished yet :(");
      try
      {
        Thread.sleep(500);
      } catch (InterruptedException e) {}
    }

    // Get and print result
    try
    {
      String result = fut.get();
      System.out.println("Result : " + result);
    } catch (InterruptedException e) {
      System.out.println("Call has been interrupted ("+ e.getMessage() + ")");
    } catch (ExecutionException e) {
      System.out.println("Error occurred : "+ e.getMessage());
    }

    // Call a 2s long function
    try
    {
      fut = proxy.asyncCall("longReply", "plaf");
    } catch (CallError e)
    {
      System.out.println("Error calling answer function : " + e.getMessage());
      return;
    }


    try
    {
      fut.get(1, TimeUnit.SECONDS);
    } catch (TimeoutException e)
    {
      System.out.println("Future.get() timeout :)");
    } catch (Exception e) {}

    try
    {
      System.out.println("But here it is : " + fut.get());
    } catch (InterruptedException e)
    {
      System.out.println("Call has been interrupted ("+ e.getMessage() + ")");
    } catch (ExecutionException e)
    {
      System.out.println("Error occurred : "+ e.getMessage());
    }


    client.close();
    client = null;
    proxy = null;
    fut = null;
    _app.stop();
    _app = null;
    // Deadlock because of network eventloop destruction, so exit.
    System.exit(0);
  }
}
