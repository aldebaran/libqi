package com.aldebaran.qimessaging;

import java.util.Map;

import junit.framework.Assert;
import junit.framework.Test;
import junit.framework.TestCase;
import junit.framework.TestSuite;

public class TupleTest extends TestCase
{

  /**
   * Create the test case
   *
   * @param testName name of the test case
   */
  public TupleTest(String testName)
  {
    super(testName);
  }

  /**
   * @return the suite of tests being tested
   */
  public static Test suite()
  {
    return new TestSuite(TupleTest.class);
  }

  public void testOutOfBoundException()
  {
    Tuple tuple = new Tuple3<String, Integer, Map<Integer, String>>();
    boolean exceptionRaised = false;

    try
    {
      tuple.<String>get(4);
    } catch (IndexOutOfBoundsException e)
    {
      exceptionRaised = true;
      System.out.println("Exception catched : " + e.getMessage());
    } catch (ClassCastException e)
    {
      Assert.assertTrue("Exception catched : " + e.getMessage(), false);
    } catch (IllegalArgumentException e)
    {
      Assert.assertTrue("Exception catched : " + e.getMessage(), false);
    } catch (IllegalAccessException e)
    {
      Assert.assertTrue("Exception catched : " + e.getMessage(), false);
    }

    Assert.assertTrue("Exception must have been thrown", exceptionRaised);
  }

  public void testTuple()
  {
    Tuple tuple = new Tuple3<String, Integer, Boolean>();
    String str  = new String("plafbim");
    Integer i   = new Integer(42);
    Boolean b   = new Boolean(true);

    try
    {
      tuple.<String>set(0, str);
    } catch (IllegalArgumentException e)
    {
      Assert.assertTrue("Exception must not be thrown", false);
    } catch (IllegalAccessException e)
    {
      Assert.assertTrue("Exception must not be thrown", false);
    }

    try
    {
      tuple.<Integer>set(1, i);
    } catch (IllegalArgumentException e)
    {
      Assert.assertTrue("Exception must not be thrown", false);
    } catch (IllegalAccessException e)
    {
      Assert.assertTrue("Exception must not be thrown", false);
    }

    try
    {
      tuple.<Boolean>set(2, b);
    } catch (IllegalArgumentException e)
    {
      Assert.assertTrue("Exception must not be thrown", false);
    } catch (IllegalAccessException e)
    {
      Assert.assertTrue("Exception must not be thrown", false);
    }

    Assert.assertTrue(true);
  }

  public void testValue()
  {
    Tuple tuple = new Tuple3<String, Integer, Boolean>("42", 42, true);
    String str  = null;
    Integer i   = null;
    Boolean b   = null;

    try
    {
      str = tuple.<String>get(0);
    } catch (IndexOutOfBoundsException e)
    {
      Assert.assertTrue("Exception must not be thrown : " + e.getMessage(), false);
    } catch (ClassCastException e)
    {
      Assert.assertTrue("Exception must not be thrown : " + e.getMessage(), false);
    } catch (IllegalArgumentException e)
    {
      Assert.assertTrue("Exception must not be thrown : " + e.getMessage(), false);
    } catch (IllegalAccessException e)
    {
      Assert.assertTrue("Exception must not be thrown : " + e.getMessage(), false);
    }

    Assert.assertTrue("String value must be '42' : " + str, str.equals("42"));


    try
    {
      i = tuple.<Integer>get(1);
    } catch (IndexOutOfBoundsException e)
    {
      Assert.assertTrue("Exception must not be thrown : " + e.getMessage(), false);
    } catch (ClassCastException e)
    {
      Assert.assertTrue("Exception must not be thrown : " + e.getMessage(), false);
    } catch (IllegalArgumentException e)
    {
      Assert.assertTrue("Exception must not be thrown : " + e.getMessage(), false);
    } catch (IllegalAccessException e)
    {
      Assert.assertTrue("Exception must not be thrown : " + e.getMessage(), false);
    }

    Assert.assertTrue("Integer value must be 42 : " + i, i == 42);

    try
    {
      b = tuple.<Boolean>get(2);
    } catch (IndexOutOfBoundsException e)
    {
      Assert.assertTrue("Exception must not be thrown : " + e.getMessage(), false);
    } catch (ClassCastException e)
    {
      Assert.assertTrue("Exception must not be thrown : " + e.getMessage(), false);
    } catch (IllegalArgumentException e)
    {
      Assert.assertTrue("Exception must not be thrown : " + e.getMessage(), false);
    } catch (IllegalAccessException e)
    {
      Assert.assertTrue("Exception must not be thrown : " + e.getMessage(), false);
    }

    Assert.assertTrue("Boolean value must be true : " + b, b);
  }

  public void testCastException()
  {
    Tuple tuple = new Tuple3<String, Integer, Map<Integer, String>>("42", 42, null);
    boolean exceptionRaised = false;

    @SuppressWarnings("unused")
    String tmp = null;
    try
    {
      tmp = tuple.<String>get(1);
    } catch (IndexOutOfBoundsException e)
    {
      Assert.assertTrue("IndexOutOfBound exception must not be thrown", false);
    } catch (ClassCastException e)
    {
      exceptionRaised = true;
      System.out.println("Exception catched : " + e.getMessage());
    } catch (IllegalArgumentException e)
    {
      Assert.assertTrue("IllegalArgumentException exception must not be thrown", false);
    } catch (IllegalAccessException e)
    {
      Assert.assertTrue("IllegalAccessException exception must not be thrown", false);
    }

    Assert.assertTrue("Exception must have been thrown", exceptionRaised);
  }


  public void testCallTuple()
  {
    ServiceDirectory sd = new ServiceDirectory();
    Session s = new Session();
    Session client = new Session();

    // Get Service directory listening url.
    String url = sd.listenUrl();

    // Create new QiMessaging generic object
    GenericObject obj = new GenericObject();

    // Get instance of ReplyService
    QimessagingService reply = new ReplyService();

    // Register method "info::(sib)(sib)", implemented from Service interface
    try
    {
      obj.advertiseMethod("info::(sib)(sib)", reply);
    } catch (Exception e2)
    {
      System.out.println("Cannot advertise method : " + e2.getMessage());
    }

    // Connect session to Service Directory
    try
    {
      s.connect(url);
    } catch (Exception e)
    {
      Assert.assertTrue("Session must be connected to Service Directory : " + e.getMessage(), false);
    }

    // Register service as serviceTest
    Assert.assertTrue("Service must be registered", s.registerService("serviceTest", obj));

    // Connect client session to service directory
    try
    {
      System.out.printf("Connecting to %s\n", url);
      client.connect(url);
    } catch (Exception e)
    {
      Assert.assertTrue("Client session must be connected to Service Directory : " + e.getMessage(), false);
    }

    // Get a proxy to serviceTest
    GenericObject proxy = null;
    try
    {
      proxy = client.service("serviceTest");
    } catch (Exception e1)
    {
      Assert.assertTrue("Cannot get serviceTest :" + e1.getMessage(), false);
    }
    Assert.assertTrue("Proxy must not be null", proxy != null);

    Tuple ret = null;
    try
    {
      ret = proxy.<Tuple>call("info", "42", 42, true);
    } catch (CallError e)
    {
      Assert.assertTrue("Call must be successful : " + e.getMessage(), false);
    }

    String str = null;
    Integer i  = null;
    Boolean b  = null;

    try
    {
      str = ret.<String>get(0);
    } catch (Exception e)
    {
      Assert.assertTrue("Exception must not be thrown : " + e.getMessage(), false);
    }

    try
    {
      i = ret.<Integer>get(1);
    } catch (Exception e)
    {
      Assert.assertTrue("Exception must not be thrown : " + e.getMessage(), false);
    }

    try
    {
      b = ret.<Boolean>get(2);
    } catch (Exception e)
    {
      Assert.assertTrue("Exception must not be thrown : " + e.getMessage(), false);
    }

    Assert.assertTrue("String value must be '42' : " + str, str.equals("42"));
    Assert.assertTrue("Integer value must be 42 : " + i, i == 42);
    Assert.assertTrue("Boolean value must be true : " + b, b);
  }

}
