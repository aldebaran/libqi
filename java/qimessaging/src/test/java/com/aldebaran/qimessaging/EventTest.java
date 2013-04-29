package com.aldebaran.qimessaging;

import junit.framework.Assert;
import junit.framework.Test;
import junit.framework.TestCase;
import junit.framework.TestSuite;

public class EventTest extends TestCase {

  private boolean callbackCalled = false;
  private int callbackParam = 0;

  /**
   * Create the test case
   *
   * @param testName name of the test case
   */
  public EventTest(String testName)
  {
    super(testName);
  }

  /**
   * @return the suite of tests being tested
   */
  public static Test suite()
  {
    return new TestSuite(EventTest.class);
  }

  public void testEvent() throws InterruptedException
  {
    ServiceDirectory sd = new ServiceDirectory();
    Session s = new Session();
    Session client = new Session();

    // Get Service directory listening url.
    String url = sd.listenUrl();

    // Create new qimessaging generic object
    GenericObject obj = new GenericObject();

    // Register event 'Fire'
    try {
      obj.advertiseEvent("fire::(i)");
    } catch (Exception e1) {
      Assert.assertTrue("Advertise event must not fail : " + e1.getMessage(), false);
    }

    // Connect session to Service Directory
    try {
      s.connect(url);
    } catch (Exception e)
    {
      Assert.assertTrue("Session must be connected to Service Directory : " + e.getMessage(), false);
    }

    // Register service as serviceTest
    Assert.assertTrue("Service must be registered", s.registerService("serviceTest", obj));

    // Connect client session to service directory
    try {
      System.out.printf("Connecting to %s\n", url);
      client.connect(url);
    } catch (Exception e)
    {
      Assert.assertTrue("Client session must be connected to Service Directory : " + e.getMessage(), false);
    }

    // Get a proxy to serviceTest
    GenericObject proxy = null;
    try {
      proxy = client.service("serviceTest");
    } catch (Exception e1) {
      Assert.assertTrue("Cannot get serviceTest :" + e1.getMessage(), false);
    }
    Assert.assertTrue("Proxy must not be null", proxy != null);

    Object callback = new Object() {
      @SuppressWarnings("unused")
      public void fireCallback(Integer i)
      {
        callbackCalled = true;
        callbackParam = i.intValue();
      }
    };

    try {
      proxy.connect("fire", "fireCallback", callback);
    } catch (Exception e) {
      Assert.assertTrue("Connect to event must succeed : " + e.getMessage(), false);
    }
    obj.emitEvent("fire", 42);

    Thread.sleep(100); // Give time for callback to be called.
    Assert.assertTrue("Event callback must have been called ", callbackCalled);
    Assert.assertTrue("Parameter value must be 42 (" + callbackParam + ")", callbackParam == 42);
  }
}
