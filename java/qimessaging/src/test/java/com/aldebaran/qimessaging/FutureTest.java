package com.aldebaran.qimessaging;

import java.util.concurrent.ExecutionException;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

import com.aldebaran.qimessaging.ServiceDirectory;
import com.aldebaran.qimessaging.Session;
import com.aldebaran.qimessaging.GenericObject;
import com.aldebaran.qimessaging.ReplyService;

import junit.framework.Test;
import junit.framework.TestCase;
import junit.framework.TestSuite;
import junit.framework.Assert;

/**
 * Integration test for QiMessaging java bindings.
 */
public class FutureTest
extends TestCase
{
  /**
   * Create the test case
   *
   * @param testName name of the test case
   */
  public FutureTest(String testName)
  {
    super(testName);
  }

  /**
   * @return the suite of tests being tested
   */
  public static Test suite()
  {
    return new TestSuite(FutureTest.class);
  }

  public Boolean onSuccessCalled;
  public Boolean onCompleteCalled;

  public void testCallback()
  {
    ServiceDirectory sd = new ServiceDirectory();
    Session s = new Session();
    Session client = new Session();

    // Get Service directory listening url.
    String url = sd.listenUrl();

    // Create new qimessaging generic object
    GenericObject obj = new GenericObject();

    // Get instance of ReplyService
    QimessagingService reply = new ReplyService();

    // Register method "reply::s(s)", implemented from Service interface
    try
    {
      obj.advertiseMethod("longReply::s(s)", reply);
    } catch (Exception e2) {
      System.out.println("Cannot advertise method : " + e2.getMessage());
    }

    // Connect session to Service Directory
    try
    {
      s.connect(url);
    } catch (Exception e)
    {
      Assert.assertTrue("Session must be connected to Service Directory : " + e.getMessage(), false);
    }

    // Register service as serviceTest
    Assert.assertTrue("Service must be registered", s.registerService("serviceTest", obj));

    // Connect client session to service directory
    try
    {
      System.out.printf("Connecting to %s\n", url);
      client.connect(url);
    } catch (Exception e)
    {
      Assert.assertTrue("Client session must be connected to Service Directory : " + e.getMessage(), false);
    }

    // Get a proxy to serviceTest
    GenericObject proxy = null;
    try
    {
      proxy = client.service("serviceTest");
    } catch (Exception e1) {
      Assert.assertTrue("Cannot get serviceTest :" + e1.getMessage(), false);
    }
    Assert.assertTrue("Proxy must not be null", proxy != null);

    this.onSuccessCalled = new Boolean(false);
    this.onCompleteCalled = new Boolean(false);
    Future<String> fut = null;
    // Call a 2s long function
    try
    {
      fut = proxy.asyncCall("longReply", "plaf");
      fut.addCallback(new Callback<String>() {

        public void onSuccess(Future<String> future, Object[] args)
        {
          onSuccessCalled = true;
          Assert.assertTrue("arg[0] must be 1", (Integer) args[0] == 1);
          Assert.assertTrue("arg[1] must be 2", (Integer) args[1] == 2);
        }

        public void onFailure(Future<String> future, Object[] args)
        {
          Assert.assertTrue("onFailure must not be called", false);
        }

        public void onComplete(Future<String> future, Object[] args)
        {
          onCompleteCalled = true;
          Assert.assertTrue("arg[0] must be 1", (Integer) args[0] == 1);
          Assert.assertTrue("arg[1] must be 2", (Integer) args[1] == 2);
        }
      }, 1, 2);
    } catch (CallError e)
    {
      System.out.println("Error calling answer function : " + e.getMessage());
      return;
    }

    try
    {
      fut.get();
    } catch (Exception e)
    {
      Assert.assertTrue("fut.wait() must not fail", false);
    }
    Assert.assertTrue("onSuccess callback must be called", onSuccessCalled);
    Assert.assertTrue("onComplete callback must be called", onCompleteCalled);
  }

  public void testLongCall()
  {
    ServiceDirectory sd = new ServiceDirectory();
    Session s = new Session();
    Session client = new Session();

    // Get Service directory listening url.
    String url = sd.listenUrl();

    // Create new qimessaging generic object
    GenericObject obj = new GenericObject();

    // Get instance of ReplyService
    QimessagingService reply = new ReplyService();

    // Register method "reply::s(s)", implemented from Service interface
    try
    {
      obj.advertiseMethod("longReply::s(s)", reply);
    } catch (Exception e2)
    {
      System.out.println("Cannot advertise method : " + e2.getMessage());
    }

    // Connect session to Service Directory
    try
    {
      s.connect(url);
    } catch (Exception e)
    {
      Assert.assertTrue("Session must be connected to Service Directory : " + e.getMessage(), false);
    }

    // Register service as serviceTest
    Assert.assertTrue("Service must be registered", s.registerService("serviceTest", obj));

    // Connect client session to service directory
    try
    {
      System.out.printf("Connecting to %s\n", url);
      client.connect(url);
    } catch (Exception e)
    {
      Assert.assertTrue("Client session must be connected to Service Directory : " + e.getMessage(), false);
    }

    // Get a proxy to serviceTest
    GenericObject proxy = null;
    try
    {
      proxy = client.service("serviceTest");
    } catch (Exception e1)
    {
      Assert.assertTrue("Cannot get serviceTest :" + e1.getMessage(), false);
    }
    Assert.assertTrue("Proxy must not be null", proxy != null);

    Future<String> fut = null;
    // Call a 2s long function
    try
    {
      fut = proxy.asyncCall("longReply", "plaf");
    } catch (CallError e)
    {
      Assert.assertTrue("Error calling answer function : " + e.getMessage(), false);
      return;
    }

    // Wait for call to finish.
    int count = 0;
    while (fut.isDone() == false)
    {
      count++;
      try
      {
        Thread.sleep(500);
      } catch (InterruptedException e) {}
    }

    Assert.assertTrue("isDone() must return false at least 3 times (" + count + ")", count > 3);

    // Get and print result
    try
    {
      String result = fut.get();
      Assert.assertTrue("result must be 'plafbim !' (" + result + ")", result.equals("plafbim !"));
    } catch (InterruptedException e)
    {
      Assert.assertTrue("Call has been interrupted ("+ e.getMessage() + ")", false);
    } catch (ExecutionException e)
    {
      Assert.assertTrue("Error occurred : "+ e.getMessage(), false);
    }
  }

  public void testCancel()
  {
    ServiceDirectory sd = new ServiceDirectory();
    Session s = new Session();
    Session client = new Session();

    // Get Service directory listening url.
    String url = sd.listenUrl();

    // Create new qimessaging generic object
    GenericObject obj = new GenericObject();

    // Get instance of ReplyService
    QimessagingService reply = new ReplyService();

    // Register method "reply::s(s)", implemented from Service interface
    try
    {
      obj.advertiseMethod("longReply::s(s)", reply);
    } catch (Exception e2)
    {
      System.out.println("Cannot advertise method : " + e2.getMessage());
    }

    // Connect session to Service Directory
    try
    {
      s.connect(url);
    } catch (Exception e)
    {
      Assert.assertTrue("Session must be connected to Service Directory : " + e.getMessage(), false);
    }

    // Register service as serviceTest
    Assert.assertTrue("Service must be registered", s.registerService("serviceTest", obj));

    // Connect client session to service directory
    try
    {
      System.out.printf("Connecting to %s\n", url);
      client.connect(url);
    } catch (Exception e)
    {
      Assert.assertTrue("Client session must be connected to Service Directory : " + e.getMessage(), false);
    }

    // Get a proxy to serviceTest
    GenericObject proxy = null;
    try
    {
      proxy = client.service("serviceTest");
    } catch (Exception e1)
    {
      Assert.assertTrue("Cannot get serviceTest :" + e1.getMessage(), false);
    }
    Assert.assertTrue("Proxy must not be null", proxy != null);

    Future<String> fut = null;
    // Call a 2s long function
    try
    {
      fut = proxy.asyncCall("longReply", "plaf");
    } catch (CallError e)
    {
      Assert.assertTrue("Error calling answer function : " + e.getMessage(), false);
      return;
    }

    // Try to cancel call
    if (fut.cancel(true) == false)
    {
      try
      {
        Thread.sleep(2000);
      } catch (InterruptedException e) {}
      return; // Test has no more sense.
    }

    boolean exceptionThrown = false;
    try
    {
      fut.get();
    }
    catch (ExecutionException e)
    {
    }
    catch (InterruptedException e)
    {
      exceptionThrown = true;
    }

    Assert.assertTrue("InterruptedException must be thrown", exceptionThrown);
    Assert.assertTrue("isCancelled must return true", fut.isCancelled());
  }

  public void testGetTimeout()
  {
    ServiceDirectory sd = new ServiceDirectory();
    Session s = new Session();
    Session client = new Session();

    // Get Service directory listening url.
    String url = sd.listenUrl();

    // Create new qimessaging generic object
    GenericObject obj = new GenericObject();

    // Get instance of ReplyService
    QimessagingService reply = new ReplyService();

    // Register method "reply::s(s)", implemented from Service interface
    try
    {
      obj.advertiseMethod("longReply::s(s)", reply);
    } catch (Exception e3)
    {
      System.out.println("Cannot advertise method : " + e3.getMessage());
    }

    // Connect session to Service Directory
    try
    {
      s.connect(url);
    } catch (Exception e)
    {
      Assert.assertTrue("Session must be connected to Service Directory : " + e.getMessage(), false);
    }

    // Register service as serviceTest
    Assert.assertTrue("Service must be registered", s.registerService("serviceTest", obj));

    // Connect client session to service directory
    try
    {
      System.out.printf("Connecting to %s\n", url);
      client.connect(url);
    } catch (Exception e)
    {
      Assert.assertTrue("Client session must be connected to Service Directory : " + e.getMessage(), false);
    }

    // Get a proxy to serviceTest
    GenericObject proxy = null;
    try
    {
      proxy = client.service("serviceTest");
    } catch (Exception e2)
    {
      Assert.assertTrue("Cannot get serviceTest :" + e2.getMessage(), false);
    }
    Assert.assertTrue("Proxy must not be null", proxy != null);

    Future<String> fut = null;
    // Call a 2s long function
    try
    {
      fut = proxy.asyncCall("longReply", "plaf");
    } catch (CallError e)
    {
      System.out.println("Error calling answer function : " + e.getMessage());
      return;
    }

    boolean hasTimeout = false;
    try
    {
      fut.get(1, TimeUnit.SECONDS);
    } catch (TimeoutException e)
    {
      hasTimeout = true;
    } catch (Exception e) {}

    Assert.assertTrue("Future.get() must timeout", hasTimeout);

    try
    {
      String ret = fut.get();
      Assert.assertTrue("InterruptedException must not be thrown", ret.equals("plafbim !"));
    } catch (InterruptedException e1)
    {
      Assert.assertTrue("InterruptedException must not be thrown", false);
    } catch (ExecutionException e1)
    {
      Assert.assertTrue("InterruptedException must not be thrown", false);
    }
  }


  public void testGetTimeoutSuccess()
  {
    ServiceDirectory sd = new ServiceDirectory();
    Session s = new Session();
    Session client = new Session();

    // Get Service directory listening url.
    String url = sd.listenUrl();

    // Create new qimessaging generic object
    GenericObject obj = new GenericObject();

    // Get instance of ReplyService
    QimessagingService reply = new ReplyService();

    // Register method "reply::s(s)", implemented from Service interface
    try
    {
      obj.advertiseMethod("longReply::s(s)", reply);
    } catch (Exception e3)
    {
      System.out.println("Cannot advertise method : " + e3.getMessage());
    }

    // Connect session to Service Directory
    try
    {
      s.connect(url);
    } catch (Exception e)
    {
      Assert.assertTrue("Session must be connected to Service Directory : " + e.getMessage(), false);
    }

    // Register service as serviceTest
    Assert.assertTrue("Service must be registered", s.registerService("serviceTest", obj));

    // Connect client session to service directory
    try
    {
      System.out.printf("Connecting to %s\n", url);
      client.connect(url);
    } catch (Exception e)
    {
      Assert.assertTrue("Client session must be connected to Service Directory : " + e.getMessage(), false);
    }

    // Get a proxy to serviceTest
    GenericObject proxy = null;
    try
    {
      proxy = client.service("serviceTest");
    } catch (Exception e2)
    {
      Assert.assertTrue("Cannot get serviceTest :" + e2.getMessage(), false);
    }
    Assert.assertTrue("Proxy must not be null", proxy != null);

    Future<String> fut = null;
    // Call a 2s long function
    try
    {
      fut = proxy.asyncCall("longReply", "plaf");
    } catch (CallError e)
    {
      System.out.println("Error calling answer function : " + e.getMessage());
      return;
    }

    String ret = null;
    try
    {
      ret = fut.get(3, TimeUnit.SECONDS);
    } catch (TimeoutException e)
    {
      Assert.assertTrue("Call must not timeout", false);
    } catch (InterruptedException e1)
    {
      Assert.assertTrue("InterruptedException must not be thrown", false);
    } catch (ExecutionException e1)
    {
      Assert.assertTrue("InterruptedException must not be thrown", false);
    }

    Assert.assertTrue("Result must be 'plafbim !' (" + ret + ")", ret.equals("plafbim !"));
  }
}
