package com.aldebaran.qimessaging;

public class QimessagingException extends Exception {

  private static final long serialVersionUID = 1L;

  /**
   * Exception thrown when error occurs during a QiMessaging operation.
   * @param e Error message.
   */
  public QimessagingException(String e)
  {
    super(e);
  }

  /**
   * Exception thrown when error occurs during a QiMessaging operation.
   */
  public QimessagingException()
  {
    super();
  }

}