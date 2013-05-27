package com.aldebaran.qimessaging;

import static org.junit.Assert.*;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class EventTest
{
  private boolean         callbackCalled = false;
  private int             callbackParam = 0;
  public Object           proxy = null;
  public Object           obj = null;
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
    java.lang.Object callback = new java.lang.Object() {
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
