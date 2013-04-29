package com.aldebaran;

import java.util.ArrayList;
import java.util.Hashtable;
import java.util.Map;

import com.aldebaran.qimessaging.CallError;
import com.aldebaran.qimessaging.GenericObject;
import com.aldebaran.qimessaging.Session;
import com.aldebaran.qimessaging.Application;

public class App
{

  public static void testFloat(GenericObject proxy)
  {
    Float answer = null;

    try
    {
      answer = (Float) proxy.<Float>call("answerFloat", 41.2);
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

  public static void testInteger(GenericObject proxy)
  {
    Integer answer = null;

    try
    {
      answer = proxy.<Integer>call("answer", 41);
    } catch (CallError e)
    {
      System.out.println("Error calling answer() :" + e.getMessage());
      return;
    } catch (NullPointerException e)
    {
      e.printStackTrace();
    }

    if (answer == null)
      System.out.println("Answer is null :(");
    else
      System.out.println("AnswerInteger : " + answer);
  }

  public static void testBoolean(GenericObject proxy)
  {
    Boolean answer = null;

    try
    {
      answer = proxy.<Boolean>call("answerBool", false);
    } catch (CallError e)
    {
      System.out.println("Error calling answerBool() :" + e.getMessage());
      return;
    }

    System.out.println("AnswerBool : " + answer);
  }

  public static void testAdd(GenericObject proxy)
  {
    Integer answer = null;

    try
    {
      answer = proxy.<Integer>call("add", 40, 2);
    } catch (CallError e)
    {
      System.out.println("Error calling add() :" + e.getMessage());
      return;
    }

    System.out.println("add : " + answer);
  }

  public static void testMap(GenericObject proxy)
  {
    Map<Integer, Boolean> abacus = new Hashtable<Integer, Boolean>();
    Map<Integer, Boolean> answer = null;

    abacus.put(1, false);
    abacus.put(2, true);
    abacus.put(4, true);

    try
    {
      answer = proxy.<Hashtable<Integer, Boolean> >call("abacus", abacus);
    }
    catch (CallError e)
    {
      System.out.println("Error calling abacus() :" + e.getMessage());
      return;
    }

    System.out.println("abacus : " + answer);
  }

  public static void testList(GenericObject proxy)
  {
    ArrayList<Integer> positions = new ArrayList<Integer>();
    ArrayList<Integer> answer = null;

    positions.add(40);
    positions.add(3);
    positions.add(3);
    positions.add(2);

    try
    {
      answer = proxy.<ArrayList<Integer> >call("echoIntegerList", positions);
    } catch (CallError e)
    {
      System.out.println("Error calling echoIntegerList() :" + e.getMessage());
      return;
    }

    System.out.println("list : " + answer);
  }

  public static void testString(GenericObject proxy)
  {
    String  str = null;

    try
    {
      str = proxy.<String>call("reply", "plaf");
    } catch (CallError e)
    {
      System.out.println("Error calling reply() :" + e.getMessage());
      return;
    }

    System.out.println("AnswerString : " + str);
  }

  public static void main( String args[] )
  {
    @SuppressWarnings("unused")
    Application app = new Application(args);
    Session client = new Session();
    String  sdAddr = "tcp://127.0.0.1:5555";


    if (args.length >= 2)
      sdAddr = args[1];

    try {
      client.connect(sdAddr);
    } catch (Exception e)
    {
      System.out.println("Cannot connect to ServiceDirectory : "+e.getMessage());
      return;
    }

    GenericObject proxy = null;
    try {
      proxy = client.service("serviceTest");
    } catch (Exception e) {
      System.out.println("Cannot get proxy on serviceTest");
      return;
    }

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
