package com.aldebaran.qimessaging;

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
public class IntegrationTest
{
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

  /**
   * Create a dumb service binding famous reply::s(s) method,
   * then connect a client session to service directory,
   * get a proxy on 'serviceTest',
   * finaly call 'reply::(s)' and check answer.
   */
  @Test
  public void testCallService()
  {
    String res = null;

    try {
      res = proxy.<String>call("reply::(s)", "plaf").get();
    } catch (Exception e)
    {
      fail("Call must succeed : " + e.getMessage());
    }

    assertNotNull(res);
    assertEquals("plafbim !", res);
  }

  /**
   * Create a dumb service binding famous reply::s(s) method,
   * then connect a client session to service directory,
   * get a proxy on 'serviceTest',
   * finaly call 'reply' without signature and check answer.
   */
  @Test
  public void testCallReplyWithoutSignature()
  {
    String res = null;

    try {
      res = proxy.<String>call("reply", "plaf").get();
    } catch (Exception e)
    {
      fail("Call must succeed : " + e.getMessage());
    }

    assertNotNull(res);
    assertEquals("plafbim !", res);
  }

  /**
   * Test multiple arguments call
   */
  @Test
  public void testWithMultipleArguments()
  {
    Integer ret = null;
    try {
      ret = proxy.<Integer>call("add", 1, 21, 20).get();
    }
    catch (Exception e)
    {
      fail("Call Error must not be thrown : " + e.getMessage());
    }

    assertEquals(42, ret.intValue());
  }
}
