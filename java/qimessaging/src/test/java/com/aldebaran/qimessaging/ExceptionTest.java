package com.aldebaran.qimessaging;

import static org.junit.Assert.*;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class ExceptionTest {

  public Boolean    exceptionThrown = false;
  public Object     proxy = null;
  public Object     obj = null;
  public Session          s = null;
  public Session          client = null;
  public ServiceDirectory sd = null;
  public Application app = null;

  @Before
  public void setUp() throws Exception
  {
    exceptionThrown = false;
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
    ob.advertiseMethod("iWillThrow::b()", reply, "Throw.");

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
    app.stop();
    app = null;
  }

  @Test
  public void test() {

    try {
      proxy.<Boolean>call("iWillThrow").get();
    } catch (Exception e) {
      exceptionThrown = true;
      System.out.println("Exception has been thrown: " + e.getMessage());
    }

    assertTrue(exceptionThrown);
  }

}
