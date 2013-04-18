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
public class ServiceTest
extends TestCase
{
  /**
   * Create the test case
   *
   * @param testName name of the test case
   */
  public ServiceTest(String testName)
  {
    super(testName);
  }

  /**
   * @return the suite of tests being tested
   */
  public static Test suite()
  {
    return new TestSuite(ServiceTest.class);
  }

  /**
   * Create a qimessaging service
   */
  public void testRegisterService()
  {
    ServiceDirectory sd = new ServiceDirectory();
    Session s = new Session();

    // Get Service directory listening url.
    String url = sd.listenUrl();

    // Create new qimessaging generic object
    GenericObject obj = new GenericObject();

    // Get instance of ReplyService
    QimessagingService reply = new ReplyService();

    // Register method "reply::s(s)", implemented from Service interface
    try {
      obj.advertiseMethod("reply::s(s)", reply);
    } catch (Exception e1) {
      System.out.println("Cannot advertise method : " + e1.getMessage());
    }

    // Connect session to Service Directory
    try {
      s.connect(url);
    } catch (Exception e)
    {
      Assert.assertTrue("Session must be connected to Service Directory : "+e.getMessage(), false);
    }

    // Register service as ServiceTestJava
    Assert.assertTrue("Service must be registered", s.registerService("ServiceTestJava", obj));
  }

  /**
   * Create a qimessaging service,
   * Get a session,
   * connect it to service directory
   * call answer() method from ServiceTestJava service
   */
  public void testCallServiceWithoutParameters()
  {
    ServiceDirectory sd = new ServiceDirectory();
    Session s = new Session();
    Session client = new Session();
    Application app = new Application(null);

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
    System.out.println("Connected.");

    // Get a proxy to ServiceTestJava
    GenericObject proxy = null;
    try {
      proxy = client.service("serviceTest");
    } catch (Exception e1) {
      Assert.assertTrue("Cannot get serviceTest :" + e1.getMessage(), false);
    }
    Assert.assertTrue("Proxy must not be null", proxy != null);
    System.out.println("Got proxy on serviceTest.");

    // Perform call and get return value
    String res = null;
    try {
      res = (String) proxy.call("answer");
    } catch (CallError e)
    {
      Assert.assertTrue("Call must succeed", false);
    }

    Assert.assertTrue("Res must not be null", res != null);
    Assert.assertTrue("Result must be 42 !", res.compareTo("42 !") == 0);
    app.stop();
  }
}
