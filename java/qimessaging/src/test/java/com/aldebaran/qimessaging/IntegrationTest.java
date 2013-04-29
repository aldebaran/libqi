package com.aldebaran.qimessaging;

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
public class IntegrationTest
extends TestCase
{
  /**
   * Create the test case
   *
   * @param testName name of the test case
   */
  public IntegrationTest( String testName )
  {
    super( testName );
  }

  /**
   * @return the suite of tests being tested
   */
  public static Test suite()
  {
    return new TestSuite( IntegrationTest.class );
  }

  /**
   * Create a dumb service binding famous reply::s(s) method,
   * then connect a client session to service directory,
   * get a proxy on 'serviceTest',
   * finaly call 'reply::(s)' and check answer.
   */
  public void testCallService()
  {
    ServiceDirectory sd = new ServiceDirectory();
    Session s = new Session();
    Session client = new Session();
    //Application app = new Application(null);

    // Get Service directory listening url.
    String url = sd.listenUrl();

    // Create new qimessaging generic object
    GenericObject obj = new GenericObject();

    // Get instance of ReplyService
    QimessagingService reply = new ReplyService();

    // Register method "reply::s(s)", implemented from Service interface
    try {
      obj.advertiseMethod("reply::s(s)", reply);
      obj.advertiseMethod("answer::s()", reply);
    } catch (Exception e2) {
      System.out.println("Cannot advertise method : " + e2.getMessage());
    }

    // Connect session to Service Directory
    try {
      s.connect(url);
    } catch (Exception e)
    {
      Assert.assertTrue("Session must be connected to Service Directory : "+e.getMessage(), false);
    }

    // Register service as ServiceTestJava
    Assert.assertTrue("Service must be registered", s.registerService("serviceTest", obj));

    // Connect client session to service directory
    try {
      client.connect(url);
    } catch (Exception e)
    {
      Assert.assertTrue("Client session must be connected to Service Directory : "+e.getMessage(), false);
    }

    // Get a proxy to ServiceTestJava
    GenericObject proxy = null;
    try {
      proxy = client.service("serviceTest");
    } catch (Exception e1) {
      Assert.assertTrue("Cannot get serviceTest :" + e1.getMessage(), false);
    }
    Assert.assertTrue("Proxy must not be null", proxy != null);

    String res = null;

    try {
      res = proxy.<String>call("reply", "plaf");
    } catch (CallError e)
    {
      Assert.assertTrue("Call must succeed withtout CallError", false);
    }
    catch (Exception e)
    {
      Assert.assertTrue("Call must succeed without std::bad_alloc : " + e.getMessage(), false);
    }

    Assert.assertTrue("Res must not be null", res != null);
    Assert.assertTrue("Result must be 'plafbim !' ("+res+")", res.compareTo("plafbim !") == 0);
  }

  /**
   * Create a dumb service binding famous reply::s(s) method,
   * then connect a client session to service directory,
   * get a proxy on 'serviceTest',
   * finaly call 'reply' without signature and check answer.
   */
  public void testCallReplyWithoutSignature()
  {
    ServiceDirectory sd = new ServiceDirectory();
    Session s = new Session();
    Session client = new Session();
    //Application app = new Application(null);

    // Get Service directory listening url.
    String url = sd.listenUrl();

    // Create new qimessaging generic object
    GenericObject obj = new GenericObject();

    // Get instance of ReplyService
    QimessagingService reply = new ReplyService();

    // Register method "reply::s(s)", implemented from Service interface
    try {
      obj.advertiseMethod("reply::s(s)", reply);
      obj.advertiseMethod("answer::s()", reply);
    } catch (Exception e2) {
      System.out.println("Cannot advertise method : " + e2.getMessage());
    }

    // Connect session to Service Directory
    try {
      s.connect(url);
    } catch (Exception e)
    {
      Assert.assertTrue("Session must be connected to Service Directory : "+e.getMessage(), false);
    }

    // Register service as ServiceTestJava
    Assert.assertTrue("Service must be registered", s.registerService("serviceTest", obj));

    // Connect client session to service directory
    try {
      client.connect(url);
      Assert.assertTrue("Client session must be connected", client.isConnected());
    } catch (Exception e)
    {
      Assert.assertTrue("Client session must be connected to Service Directory : "+e.getMessage(), false);
    }

    // Get a proxy to ServiceTestJava
    GenericObject proxy = null;
    try {
      proxy = client.service("serviceTest");
    } catch (Exception e1) {
      Assert.assertTrue("Cannot get serviceTest :" + e1.getMessage(), false);
    }
    Assert.assertTrue("Proxy must not be null", proxy != null);

    String res = null;

    try {
      res = proxy.<String>call("reply", "plaf");
    } catch (CallError e)
    {
      Assert.assertTrue("Call must succeed withtout CallError", false);
    }
    catch (Exception e)
    {
      Assert.assertTrue("Call must succeed : " + e.getMessage(), false);
    }

    Assert.assertTrue("Res must not be null", res != null);
    Assert.assertTrue("Result must be 'plafbim !' ("+res+")", res.compareTo("plafbim !") == 0);
  }

  /**
   * Test multiple arguments call
   */
  public void testWithTwoArguments()
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
    try {
      obj.advertiseMethod("add::i(iii)", reply);
    } catch (Exception e2) {
      System.out.println("Cannot advertise method : " + e2.getMessage());
    }

    // Connect session to Service Directory
    try {
      s.connect(url);
    } catch (Exception e)
    {
      Assert.assertTrue("Session must be connected to Service Directory : " + e.getMessage(), false);
    }

    // Register service as ServiceTestJava
    Assert.assertTrue("Service must be registered", s.registerService("serviceTest", obj));

    // Connect client session to service directory
    try {
      System.out.printf("Connecting to %s\n", url);
      client.connect(url);
    } catch (Exception e)
    {
      Assert.assertTrue("Client session must be connected to Service Directory : " + e.getMessage(), false);
    }

    // Get a proxy to ServiceTestJava
    GenericObject proxy = null;
    try {
      proxy = client.service("serviceTest");
    } catch (Exception e1) {
      Assert.assertTrue("Cannot get serviceTest :" + e1.getMessage(), false);
    }
    Assert.assertTrue("Proxy must not be null", proxy != null);

    Integer ret = null;
    try {
      ret = proxy.<Integer>call("add", 1, 21, 20);
    }
    catch (CallError e)
    {
      Assert.assertTrue("Call Error must not be thrown : " + e.getMessage(), false);
    }

    Assert.assertTrue("Result must be 42 ("+ret+")", ret == 42);
  }
}
