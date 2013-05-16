package com.aldebaran.qimessaging;

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
public class IntegrationTest
{
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

    // Get instance of ReplyService
    QimessagingService reply = new ReplyService();

    // Register event 'Fire'
    obj.advertiseSignal("fire::(i)");
    obj.advertiseMethod("reply::s(s)", reply);
    obj.advertiseMethod("answer::s()", reply);
    obj.advertiseMethod("add::i(iii)", reply);

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
      res = proxy.<String>call("reply::(s)", "plaf");
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
      res = proxy.<String>call("reply", "plaf");
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
      ret = proxy.<Integer>call("add", 1, 21, 20);
    }
    catch (CallError e)
    {
      fail("Call Error must not be thrown : " + e.getMessage());
    }

    assertEquals(42, ret.intValue());
  }
}
