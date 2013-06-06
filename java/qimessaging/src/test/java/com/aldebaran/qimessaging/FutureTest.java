package com.aldebaran.qimessaging;

import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

import com.aldebaran.qimessaging.ServiceDirectory;
import com.aldebaran.qimessaging.Session;
import com.aldebaran.qimessaging.ReplyService;
import com.aldebaran.qimessaging.Object;

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
  public Object           proxy = null;
  public Object           obj = null;
  public Session          s = null;
  public Session          client = null;
  public ServiceDirectory sd = null;
  public boolean          onSuccessCalled = false;
  public boolean          onCompleteCalled = false;

  @Before
  public void setUp() throws Exception
  {
    onSuccessCalled = false;
    onCompleteCalled = false;
    app = new Application(null);
    sd = new ServiceDirectory();
    s = new Session();
    client = new Session();

    // Get Service directory listening url.
    String url = sd.listenUrl();

    // Create new QiMessaging generic object
    GenericObjectBuilder ob = new GenericObjectBuilder();

    // Get instance of ReplyService
    QimessagingService reply = new ReplyService();

    // Register event 'Fire'
    ob.advertiseSignal("fire::(i)");
    ob.advertiseMethod("reply::s(s)", reply, "Concatenate given argument with 'bim !'");
    ob.advertiseMethod("answer::s()", reply, "Return given argument");
    ob.advertiseMethod("add::i(iii)", reply, "Return sum of arguments");
    ob.advertiseMethod("info::(sib)(sib)", reply, "Return a tuple containing given arguments");
    ob.advertiseMethod("answer::i(i)", reply, "Return given parameter plus 1");
    ob.advertiseMethod("answerFloat::f(f)", reply, "Return given parameter plus 1");
    ob.advertiseMethod("answerBool::b(b)", reply, "Flip given parameter and return it");
    ob.advertiseMethod("abacus::{ib}({ib})", reply, "Flip all booleans in map");
    ob.advertiseMethod("echoFloatList::[m]([f])", reply, "Return the exact same list");
    ob.advertiseMethod("createObject::o()", reply, "Return a test object");
    ob.advertiseMethod("longReply::s(s)", reply, "Sleep 2s, then return given argument + 'bim !'");

    // Connect session to Service Directory
    s.connect(url).sync();

    // Register service as serviceTest
    obj = ob.object();
    assertTrue("Service must be registered", s.registerService("serviceTest", obj));

    // Connect client session to service directory
    client.connect(url);

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
    Object proxy = null;
    Future<String> fut = null;


    // Get a proxy to serviceTest
    try
    {
      proxy = client.service("serviceTest");
    } catch (Exception e1)
    {
      fail("Cannot get serviceTest :" + e1.getMessage());
    }

    // Call a 2s long function
    try
    {
      fut = proxy.call("longReply", "plaf");
      fut.addCallback(new Callback<String>() {

        public void onSuccess(Future<String> future, java.lang.Object[] args)
        {
          onSuccessCalled = true;
          assertEquals(1, args[0]);
          assertEquals(2, args[1]);
        }

        public void onFailure(Future<String> future, java.lang.Object[] args)
        {
          fail("onFailure must not be called");
        }

        public void onComplete(Future<String> future, java.lang.Object[] args)
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
      fut = proxy.call("longReply", "plaf");
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
  public void testGetTimeout()
  {
    Future<String> fut = null;

    // Call a 2s long function
    try
    {
      fut = proxy.call("longReply", "plaf");
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
      fut = proxy.call("longReply", "plaf");
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
