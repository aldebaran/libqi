package com.aldebaran.qimessaging;

import java.util.ArrayList;
import java.util.Hashtable;
import java.util.List;
import java.util.Map;

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
public class TypeTest
extends TestCase
{
  /**
   * Create the test case
   *
   * @param testName name of the test case
   */
  public TypeTest(String testName)
  {
    super(testName);
  }

  /**
   * @return the suite of tests being tested
   */
  public static Test suite()
  {
    return new TestSuite(TypeTest.class);
  }

  /**
   * Test String conversion
   */
  public void testString()
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

    String ret = null;
    try {
      ret = proxy.<String>call("reply", "plaf");
    }
    catch (CallError e)
    {
      Assert.assertTrue("Call Error must not be thrown : " + e.getMessage(), false);
    }

    Assert.assertTrue("Result must be 'plafbim !'", ret.compareTo("plafbim !") == 0);

    try {
      ret = proxy.<String>call("answer");
    }
    catch (CallError e)
    {
      Assert.assertTrue("Call Error must not be thrown : " + e.getMessage(), false);
    }

    Assert.assertTrue("Result must be '42 !'", ret.compareTo("42 !") == 0);
  }

  /**
   * Test Integer conversion
   */
  public void testInt()
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
      obj.advertiseMethod("answer::i(i)", reply);
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

    Integer ret = null;
    try {
      ret = proxy.<Integer>call("answer", 41);
    }
    catch (CallError e)
    {
      Assert.assertTrue("Call Error must not be thrown : " + e.getMessage(), false);
    }

    Assert.assertTrue("Result must be 42", ret == 42);
  }

  /**
   * Test Float conversion
   */
  public void testFloat()
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
      obj.advertiseMethod("answerFloat::f(f)", reply);
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

    Float ret = null;
    try {
      ret = proxy.<Float>call("answerFloat", 41.2f);
    }
    catch (CallError e)
    {
      Assert.assertTrue("Call Error must not be thrown : " + e.getMessage(), false);
    }

    Assert.assertTrue("Result must be 42.2 ("+ret+")", ret == 42.2f);
  }

  /**
   * Test Boolean conversion
   */
  public void testBoolean()
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
      obj.advertiseMethod("answerBool::b(b)", reply);
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

    Boolean ret = null;
    try {
      ret = proxy.<Boolean>call("answerBool", false);
    }
    catch (CallError e)
    {
      Assert.assertTrue("Call Error must not be thrown : " + e.getMessage(), false);
    }

    Assert.assertTrue("Result must be true", ret);
  }

  /**
   * Test Map conversion
   */
  public void testMap()
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
      obj.advertiseMethod("abacus::{mm}({mm})", reply);
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


    Map<Integer, Boolean> args = new Hashtable<Integer, Boolean>();

    Map<Integer, Boolean> ret = null;
    try {
      ret = proxy.<Hashtable<Integer, Boolean> >call("abacus", args);
    }
    catch (CallError e)
    {
      Assert.assertTrue("Call Error must not be thrown : " + e.getMessage(), false);
    }

    System.out.println("Ret : " + ret);
    Assert.assertTrue("Result must be empty", ret.size() == 0);
  }

  /**
   * Test Map conversion
   */
  public void testIntegerBooleanMap()
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
      obj.advertiseMethod("abacus::{mm}({mm})", reply);
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


    Map<Integer, Boolean> args = new Hashtable<Integer, Boolean>();
    args.put(4, true);
    args.put(3, false);
    args.put(2, false);
    args.put(1, true);

    Map<Integer, Boolean> ret = null;
    try {
      ret = proxy.<Hashtable<Integer, Boolean> >call("abacus", args);
    }
    catch (CallError e)
    {
      Assert.assertTrue("Call Error must not be thrown : " + e.getMessage(), false);
    }

    System.out.println("Ret : " + ret);
    Assert.assertTrue("Result must be false", ret.get(1) == false);
    Assert.assertTrue("Result must be true", ret.get(2) == true);
    Assert.assertTrue("Result must be true", ret.get(3) == true);
    Assert.assertTrue("Result must be false", ret.get(4) == false);
  }

  /**
   * Test List conversion
   */
  public void testList()
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
      obj.advertiseMethod("echoFloatList::[m]([m])", reply);
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


    List<Float> args = new ArrayList<Float>();

    List<Float> ret = null;
    try {
      ret = proxy.<ArrayList<Float> >call("echoFloatList", args);
    }
    catch (CallError e)
    {
      Assert.assertTrue("Call Error must not be thrown : " + e.getMessage(), false);
    }

    Assert.assertTrue("Result must be empty", ret.size() == 0);
  }

  /**
   * Test List conversion
   */
  public void testIntegerList()
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
      obj.advertiseMethod("echoFloatList::[m]([m])", reply);
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


    List<Float> args = new ArrayList<Float>();
    args.add(13.3f);
    args.add(1342.3f);
    args.add(13.4f);
    args.add(1.0f);
    args.add(0.1f);

    List<Float> ret = null;
    try {
      ret = proxy.<ArrayList<Float> >call("echoFloatList", args);
    }
    catch (CallError e)
    {
      Assert.assertTrue("Call Error must not be thrown : " + e.getMessage(), false);
    }

    System.out.println(ret + " VS " + args);
    Assert.assertTrue("Result must be equals to arguments", ret.equals(args));
  }
}
