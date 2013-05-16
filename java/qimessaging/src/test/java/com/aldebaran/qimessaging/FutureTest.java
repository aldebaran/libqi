package com.aldebaran.qimessaging;

import java.util.concurrent.ExecutionException;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

import com.aldebaran.qimessaging.ServiceDirectory;
import com.aldebaran.qimessaging.Session;
import com.aldebaran.qimessaging.GenericObject;
import com.aldebaran.qimessaging.ReplyService;

import static org.junit.Assert.*;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

/**
 * Integration test for QiMessaging java bindings.
 */
public class FutureTest
{
  public Application      app = null;
  public ServiceDirectory sd = null;
  public Session          s = null;
  public Session          client = null;
  public GenericObject    proxy = null;

  public Boolean onSuccessCalled;
  public Boolean onCompleteCalled;

  @Before
  public void setUp() throws Exception
  {
    app = new Application(null);
    sd = new ServiceDirectory();
    s = new Session();
    client = new Session();

    // Get Service directory listening url.
    String url = sd.listenUrl();

    // Create new qimessaging generic object
    GenericObject obj = new GenericObject();

    // Get instance of ReplyService
    QimessagingService reply = new ReplyService();

    // Register method "reply::s(s)", implemented from Service interface
    obj.advertiseMethod("longReply::s(s)", reply);

    // Connect session to Service Directory
    s.connect(url).sync();

    // Register service
    s.registerService("serviceTest", obj);

    // Connect client session to service directory
    client.connect(url).sync();

    // Get a proxy to serviceTest
    proxy = client.service("serviceTest");
    assertNotNull(proxy);
  }

  @After
  public void tearDown()
  {
    s.close();
    client.close();

    s = null;
    client = null;
    sd = null;
    app.stop();
    app = null;
  }

  @Test
  public void testCallback()
  {
    Future<String> fut = null;

    // Call a 2s long function
    try
    {
      fut = proxy.asyncCall("longReply", "plaf");
      fut.addCallback(new Callback<String>() {

        public void onSuccess(Future<String> future, Object[] args)
        {
          onSuccessCalled = true;
          assertEquals(1, args[0]);
          assertEquals(2, args[1]);
        }

        public void onFailure(Future<String> future, Object[] args)
        {
          fail("onFailure must not be called");
        }

        public void onComplete(Future<String> future, Object[] args)
        {
          onCompleteCalled = true;
          assertEquals(1, args[0]);
          assertEquals(2, args[1]);
        }
      }, 1, 2);
    } catch (CallError e)
    {
      fail("Error calling answer function : " + e.getMessage());
    }

    try
    {
      fut.get();
    } catch (Exception e)
    {
      fail("fut.get() must not fail");
    }
    assertTrue(onSuccessCalled);
    assertTrue(onCompleteCalled);
  }

  @Test
  public void testLongCall()
  {
    Future<String> fut = null;

    // Call a 2s long function
    try
    {
      fut = proxy.asyncCall("longReply", "plaf");
    } catch (CallError e)
    {
      fail("Error calling answer function : " + e.getMessage());
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

    assertTrue("isDone() must return false at least 3 times (" + count + ")", count > 3);

    // Get and print result
    try
    {
      String result = fut.get();
      assertEquals("plafbim !", result);
    } catch (Exception e)
    {
      fail("Call has been interrupted ("+ e.getMessage() + ")");
    }
  }

  @Test
  public void testCancel()
  {
    Future<String> fut = null;

    // Call a 2s long function
    try
    {
      fut = proxy.asyncCall("longReply", "plaf");
    } catch (CallError e)
    {
      fail("Error calling answer function : " + e.getMessage());
    }

    // Try to cancel call
    if (fut.cancel() == false)
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

    assertTrue("InterruptedException must be thrown", exceptionThrown);
    assertTrue("isCancelled must return true", fut.isCancelled());
  }

  @Test
  public void testGetTimeout()
  {
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

    assertTrue("Future.get() must timeout", hasTimeout);

    try
    {
      String ret = fut.get();
      assertEquals("plafbim !", ret);
    } catch (Exception e1)
    {
      fail("InterruptedException must not be thrown");
    }
  }

  @Test
  public void testGetTimeoutSuccess()
  {
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
      fail("Call must not timeout");
    } catch (Exception e1)
    {
      fail("InterruptedException must not be thrown");
    }

    assertEquals("plafbim !", ret);
  }
}
