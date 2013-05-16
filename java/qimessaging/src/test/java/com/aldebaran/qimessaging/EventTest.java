package com.aldebaran.qimessaging;

import static org.junit.Assert.*;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class EventTest
{
  private boolean         callbackCalled = false;
  private int             callbackParam = 0;
  public GenericObject    proxy = null;
  public GenericObject    obj = null;
  public Session          s = null;
  public Session          client = null;
  public ServiceDirectory sd = null;

  @Before
  public void setUp() throws Exception
  {
    sd = new ServiceDirectory();
    s = new Session();
    client = new Session();

    // Get Service directory listening url.
    String url = sd.listenUrl();

    // Create new QiMessaging generic object
    obj = new GenericObject();

    // Register event 'Fire'
    obj.advertiseSignal("fire::(i)");

    // Connect session to Service Directory
    s.connect(url).sync();

    // Register service as serviceTest
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
    obj = null;
    proxy = null;

    s.close();
    client.close();

    s = null;
    client = null;
    sd = null;
  }

  @Test
  public void testEvent() throws InterruptedException
  {

    @SuppressWarnings("unused")
    Object callback = new Object() {
      public void fireCallback(Integer i)
      {
        callbackCalled = true;
        callbackParam = i.intValue();
      }
    };

    try {
      proxy.connect("fire", "fireCallback", callback);
    } catch (Exception e) {
      fail("Connect to event must succeed : " + e.getMessage());
    }
    obj.post("fire", 42);

    Thread.sleep(100); // Give time for callback to be called.
    assertTrue("Event callback must have been called ", callbackCalled);
    assertTrue("Parameter value must be 42 (" + callbackParam + ")", callbackParam == 42);
  }
}
