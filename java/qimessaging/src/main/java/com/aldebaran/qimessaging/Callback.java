package com.aldebaran.qimessaging;

public interface Callback<T>
{

  public void onSuccess(Future<T> future, Object[] args);
  public void onFailure(Future<T> future, Object[] args);
  public void onComplete(Future<T> future, Object[] args);

}
