package com.aldebaran;

import com.aldebaran.qimessaging.Application;
import com.aldebaran.qimessaging.QimessagingException;
import com.aldebaran.qimessaging.Session;
import com.aldebaran.qimessaging.QimessagingService;
import com.aldebaran.qimessaging.GenericObject;

public class App
{
  public static void main( String[] args )
  {
    Application app = new Application(args);
    String sdAddr = "tcp://127.0.0.1:5555";
    Session s = new Session();
    QimessagingService service = new ReplyService();
    GenericObject obj = new GenericObject();

    if (args.length >= 2)
      sdAddr = args[0];

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
      obj.advertiseMethod("reply::s(s)", service);
      obj.advertiseMethod("longReply::s(s)", service);
      obj.advertiseMethod("answer::i(i)", service);
      obj.advertiseMethod("answerFloat::f(f)", service);
      obj.advertiseMethod("answerBool::b(b)", service);
      obj.advertiseMethod("echoIntegerList::[m]([m])", service);
      obj.advertiseMethod("add::i(ii)", service);
      obj.advertiseMethod("abacus::{ib}({ib})", service);
      obj.advertiseMethod("info::(sib)(sib)", service);
      obj.advertiseMethod("triggerFireEvent::(i)", service);
    } catch (QimessagingException e1) {
      System.out.println("Cannot advertise method : " + e1.getMessage());
    }

    ReplyService rs = (ReplyService) service;
    rs.setObj(obj);

    s.registerService("serviceTest", obj);

    System.out.println("Ready.");
    app.run();
  }
}
