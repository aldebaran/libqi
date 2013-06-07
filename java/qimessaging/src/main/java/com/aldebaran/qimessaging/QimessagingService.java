package com.aldebaran.qimessaging;

/**
 * @since 1.20
 * Implement this interface to create a Qimessaging service.
 */
public abstract class QimessagingService {

  protected Object self;

  public void init(Object self)
  {
    this.self = self;
  }

}
