package com.aldebaran.qimessaging;

@SuppressWarnings("serial")
public final class ConnectionError extends Exception
{

  public ConnectionError(String e)
  {
    super(e);
  }

  public ConnectionError()
  {
  }

}
