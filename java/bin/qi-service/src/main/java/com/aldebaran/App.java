package com.aldebaran;

import com.aldebaran.qimessaging.Application;
import com.aldebaran.qimessaging.GenericObjectBuilder;
import com.aldebaran.qimessaging.QimessagingException;
import com.aldebaran.qimessaging.Object;
import com.aldebaran.qimessaging.Session;
import com.aldebaran.qimessaging.QimessagingService;

public class App
{
  public static void main( String[] args )
  {
    Application app = new Application(args);
    String sdAddr = "tcp://127.0.0.1:9559";
    Session s = new Session();
    QimessagingService service = new ReplyService();
    GenericObjectBuilder ob = new GenericObjectBuilder();

    if (args.length >= 1)
      sdAddr = args[0];

    System.out.println("Connecting to " + sdAddr);
    try
    {
      s.connect(sdAddr);
    } catch (Exception e)
    {
      System.out.println("Cannot connect to ServiceDirectory : " + e.getMessage());
      return;
    }

    try
    {
      ob.advertiseMethod("reply::s(s)", service, "Concatenate given argument with 'bim !'");
      ob.advertiseMethod("answer::s()", service, "Return given argument");
      ob.advertiseMethod("add::i(ii)", service, "Return sum of arguments");
      ob.advertiseMethod("info::(sib)(sib)", service, "Return a tuple containing given arguments");
      ob.advertiseMethod("answer::i(i)", service, "Return given parameter plus 1");
      ob.advertiseMethod("answerFloat::f(f)", service, "Return given parameter plus 1");
      ob.advertiseMethod("answerBool::b(b)", service, "Flip given parameter and return it");
      ob.advertiseMethod("abacus::{ib}({ib})", service, "Flip all booleans in map");
      ob.advertiseMethod("echoFloatList::[m]([f])", service, "Return the exact same list");
      ob.advertiseMethod("createObject::o()", service, "Return a test object");
      ob.advertiseMethod("triggerFireEvent::(i)", service, "Trigger Fire event");
      ob.advertiseMethod("echoIntegerList::[i]([i])", service, "Trigger Fire event");
    } catch (QimessagingException e1) {
      System.out.println("Cannot advertise method : " + e1.getMessage());
    }
    try {
      ob.advertiseSignal("fire::(i)");
    } catch (Exception e) {
      System.out.println("Cannot advertise event : " + e.getMessage());
    }

    Object obj = ob.object();
    service.init(obj);

    try {
      s.registerService("serviceTest", obj);
    } catch (Exception e)
    {
      System.out.println("Cannot register service serviceTest : " + e.getMessage());
      return;
    }

    System.out.println("Ready.");
    app.run();
  }
}
