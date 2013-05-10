package com.aldebaran.qimessaging;

import com.aldebaran.qimessaging.ServiceDirectory;
import com.aldebaran.qimessaging.Session;
import com.aldebaran.qimessaging.GenericObject;

import junit.framework.Test;
import junit.framework.TestCase;
import junit.framework.TestSuite;
import junit.framework.Assert;

/**
 * Integration test for QiMessaging java bindings.
 */
public class ClientTest
extends TestCase
{
  /**
   * Create the test case
   *
   * @param testName name of the test case
   */
  public ClientTest(String testName)
  {
    super(testName);
  }

  /**
   * @return the suite of tests being tested
   */
  public static Test suite()
  {
    return new TestSuite( ClientTest.class );
  }

  /**
   * Create a qimessaging session,
   * Connect to Service Directory,
   */
  public void testConnection()
  {
    ServiceDirectory sd = new ServiceDirectory();
    Session s = new Session();

    // Get Service directory listening url.
    String url = sd.listenUrl();

    // Try to connect session to service directory.
    try {
      s.connect(url);
    } catch (Exception e)
    {
      Assert.assertTrue("Connection to service directory must succeed : "+e.getMessage(), false);
    }
  }

  /**
   * Create a qimessaging session,
   * Connect to Service Directory,
   * Close session,
   * Close service directory.
   */
  public void testDisconnection()
  {
    ServiceDirectory sd = new ServiceDirectory();
    Session s = new Session();

    // Get Service directory listening url.
    String url = sd.listenUrl();

    // Try to connect session to service directory.
    try {
      s.connect(url).sync();
    } catch (Exception e)
    {
      System.out.printf("Cannot connect to service directory (%s) : %s", url, e.getMessage());
      assert(false);
    }

    //FIXME: Segfault.
    //s.close();
    //sd.close();
  }

  /**
   * Test Session constructor
   */
  public void testSessionConstructor()
  {
    ServiceDirectory sd = new ServiceDirectory();
    Session s = null;

    try {
      s = new Session(sd.listenUrl());
    }
    catch (Exception e)
    {
      Assert.assertTrue("Session must be connected to ServiceDirectory", false);
    }

    Assert.assertTrue("Session must be connected to ServiceDirectory", s.isConnected());
  }

  /**
   * Create a qimessaging session,
   * then connect session to service directory,
   * get a proxy on 'serviceTest',
   * finaly call 'reply' and check answer.
   */
  public void testExplicitCallReply()
  {
    String  answer = null;
    Session s = new Session();
    ServiceDirectory sd = new ServiceDirectory();
    GenericObject obj = new GenericObject();
    QimessagingService service = new ReplyService();


    // Get Service directory listening url.
    String url = sd.listenUrl();

    // Try to connect session to service directory.
    try {
      s.connect(url);
    } catch (Exception e)
    {
      Assert.assertTrue("Session must be connected to ServiceDirectory", false);
    }


    // Register service
    try {
      obj.advertiseMethod("reply::s(s)", service);
    } catch (Exception e2) {
      System.out.println("Cannot advertise method : " + e2.getMessage());
    }
    s.registerService("serviceTest", obj);

    // Get a proxy on serviceTest service.
    GenericObject proxy = null;
    try {
      proxy = s.service("serviceTest");
    } catch (Exception e1) {
      Assert.assertTrue("Cannot get serviceTest :" + e1.getMessage(), false);
    }
    Assert.assertTrue("Proxy must not be null", proxy != null);

    // Call 'reply' method
    try {
      answer = proxy.<String>call("reply", "foo");

    } catch (CallError e)
    {
      Assert.assertTrue("Call must succeed : "+e.getMessage(), false);
    } catch (Exception e) {
      Assert.assertTrue("Call must succeed : "+e.getMessage(), false);
    }

    // Assert answer is 'plafbim'
    Assert.assertTrue("Answer must be 'foobim !' ("+answer+")", answer.equals("foobim !"));
  }
}
