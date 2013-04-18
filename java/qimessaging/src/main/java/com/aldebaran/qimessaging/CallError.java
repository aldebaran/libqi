package com.aldebaran.qimessaging;

@SuppressWarnings("serial")
public class CallError extends Exception
{

  /**
   * Exception thrown when error occurs during a call.
   * @param e Error message.
   */
  public CallError(String e)
  {
    super(e);
  }

  /**
   * Exception thrown when error occurs during a call.
   */
  public CallError()
  {
    super();
  }

}
