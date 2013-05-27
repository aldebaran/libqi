package com.aldebaran;

import java.util.ArrayList;
import java.util.Hashtable;
import java.util.Map;

import com.aldebaran.qimessaging.CallError;
import com.aldebaran.qimessaging.Object;
import com.aldebaran.qimessaging.Session;
import com.aldebaran.qimessaging.Application;

public class App
{

  public static void testFloat(Object proxy)
  {
    Float answer = null;

    try
    {
      answer = proxy.<Float>call("answerFloat", 41.2).get();
    } catch (CallError e)
    {
      System.out.println("Error calling answerFloat() :" + e.getMessage());
      return;
    } catch (Exception e)
    {
      System.out.println("Unexpected error calling answerFloat() : " + e.getMessage());
      return;
    }

    System.out.println("AnswerFloat : " + answer);
  }

  public static void testInteger(Object proxy)
  {
    Integer answer = null;

    try
    {
      answer = proxy.<Integer>call("answer", 41).get();
    } catch (Exception e)
    {
      System.out.println("Error calling answer() :" + e.getMessage());
      return;
    }

    if (answer == null)
      System.out.println("Answer is null :(");
    else
      System.out.println("AnswerInteger : " + answer);
  }

  public static void testBoolean(Object proxy)
  {
    Boolean answer = null;

    try
    {
      answer = proxy.<Boolean>call("answerBool", false).get();
    } catch (Exception e)
    {
      System.out.println("Error calling answerBool() :" + e.getMessage());
      return;
    }

    System.out.println("AnswerBool : " + answer);
  }

  public static void testAdd(Object proxy)
  {
    Integer answer = null;

    try
    {
      answer = proxy.<Integer>call("add", 40, 2).get();
    } catch (Exception e)
    {
      System.out.println("Error calling add() :" + e.getMessage());
      return;
    }

    System.out.println("add : " + answer);
  }

  public static void testMap(Object proxy)
  {
    Map<Integer, Boolean> abacus = new Hashtable<Integer, Boolean>();
    Map<Integer, Boolean> answer = null;

    abacus.put(1, false);
    abacus.put(2, true);
    abacus.put(4, true);

    try
    {
      answer = proxy.<Hashtable<Integer, Boolean> >call("abacus", abacus).get();
    }
    catch (Exception e)
    {
      System.out.println("Error calling abacus() :" + e.getMessage());
      return;
    }

    System.out.println("abacus : " + answer);
  }

  public static void testList(Object proxy)
  {
    ArrayList<Integer> positions = new ArrayList<Integer>();
    ArrayList<Integer> answer = null;

    positions.add(40);
    positions.add(3);
    positions.add(3);
    positions.add(2);

    try
    {
      answer = proxy.<ArrayList<Integer> >call("echoIntegerList", positions).get();
    } catch (Exception e)
    {
      System.out.println("Error calling echoIntegerList() :" + e.getMessage());
      return;
    }

    System.out.println("list : " + answer);
  }

  public static void testString(Object proxy)
  {
    String  str = null;

    try
    {
      str = proxy.<String>call("reply", "plaf").get();
    } catch (Exception e)
    {
      System.out.println("Error calling reply() :" + e.getMessage());
      return;
    }

    System.out.println("AnswerString : " + str);
  }

  public static void testObject(Object proxy)
  {
    Object ro = null;
    try
    {
      ro = proxy.<Object>call("createObject").get();
    } catch (Exception e)
    {
      System.out.println("Call failed: " + e.getMessage());
      return;
    }

    String prop = (String) ro.property("name");
    System.out.println("Property : " + prop);
  }

  public static void main( String args[] )
  {
    @SuppressWarnings("unused")
    Application app = new Application(args);
    Session client = new Session();
    String  sdAddr = "tcp://127.0.0.1:9559";


    if (args.length >= 1)
      sdAddr = args[0];

    try {
      client.connect(sdAddr);
    } catch (Exception e)
    {
      System.out.println("Cannot connect to ServiceDirectory : "+e.getMessage());
      return;
    }

    Object proxy = null;
    try {
      proxy = client.service("serviceTest");
    } catch (Exception e) {
      System.out.println("Cannot get proxy on serviceTest");
      return;
    }


    testObject(proxy);
    testMap(proxy);
    testList(proxy);
    testString(proxy);

    testInteger(proxy);
    testFloat(proxy);
    testBoolean(proxy);
    testAdd(proxy);

    EventTester t = new EventTester();
    t.testEvent(proxy);

    System.exit(0);
  }
}
