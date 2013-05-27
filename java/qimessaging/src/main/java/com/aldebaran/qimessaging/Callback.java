package com.aldebaran.qimessaging;

public interface Callback<T>
{

  /**
   * Called when future completes successfully.
   * @param future Successful Future
   * @param args Arguments given to Future.addCallback() method.
   */
  public void onSuccess(Future<T> future, java.lang.Object[] args);

  /**
   * Called when future ends with error.
   * @param future Future with error
   * @param args Arguments given to Future.addCallback() method.
   */
  public void onFailure(Future<T> future, java.lang.Object[] args);

  /**
   * Called when future completes.
   * @param future Completed Future
   * @param args Arguments given to Future.addCallback() method.
   */
  public void onComplete(Future<T> future, java.lang.Object[] args);

}
