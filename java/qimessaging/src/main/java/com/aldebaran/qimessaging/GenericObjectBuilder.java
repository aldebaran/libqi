package com.aldebaran.qimessaging;

import java.lang.reflect.Method;

public class GenericObjectBuilder {

  static
  {
    // Loading native C++ libraries.
    if (!EmbeddedTools.LOADED_EMBEDDED_LIBRARY)
    {
      EmbeddedTools loader = new EmbeddedTools();
      loader.loadEmbeddedLibraries();
    }
  }

  private long _p;

  private static native long   create();
  private static native void   destroy(long pObject);
  private static native java.lang.Object object(long pObjectBuilder);
  private static native long   advertiseMethod(long pObjectBuilder, String method, java.lang.Object instance, String className, String description);
  private static native long   advertiseSignal(long pObjectBuilder, String eventSignature);
  private static native long   advertiseProperty(long pObjectBuilder, String name, Class<?> propertyBase);

  public GenericObjectBuilder()
  {
    _p = GenericObjectBuilder.create();
  }

  /**
   * Bind method from a qimessaging.service to GenericObject.
   * @param methodSignature Signature of method to bind.
   * @param service Service implementing method.
   * @throws Exception on error.
   */
  public void advertiseMethod(String methodSignature, QimessagingService service, String description) throws QimessagingException
  {
    Class<?extends java.lang.Object> c = service.getClass();
    Method[] methods = c.getDeclaredMethods();

    if (_p == 0)
      throw new QimessagingException("Invalid object.\n");

    for (Method method : methods)
    {
      String className = service.getClass().toString();
      className = className.substring(6); // Remove "class "
      className = className.replace('.', '/');

      // If method name match signature
      if (methodSignature.contains(method.getName()) == true)
      {
        if (GenericObjectBuilder.advertiseMethod(_p, methodSignature, service, className, description) == 0)
          throw new QimessagingException("Cannot register method " + methodSignature);
        return;
      }
    }
  }

  /**
   * Advertise an signal with its callback signature.
   * @param signalSignature Signature of available callback.
   * @throws Exception If GenericObject is not initialized internally.
   */
  public void advertiseSignal(String signalSignature) throws Exception
  {
    if (_p == 0)
      throw new Exception("Invalid object");
    GenericObjectBuilder.advertiseSignal(_p, signalSignature);
  }

  public void advertiseProperty(String name, Class<?> propertyBase) throws QimessagingException
  {
    if (_p == 0)
      throw new QimessagingException("Invalid object");
    if (GenericObjectBuilder.advertiseProperty(_p, name, propertyBase) <= 0)
      throw new QimessagingException("Cannot advertise " + name + " property");
  }

  /**
   * Instantiate new Object after builder template.
   * @see Object
   * @return Object
   */
  public Object object()
  {
    return (Object) GenericObjectBuilder.object(_p);
  }

  /**
   * Called on garbage collection, ensure that C++ ObjectBuilder is destroyed too.
   */
  protected void finalize()
  {
    GenericObjectBuilder.destroy(_p);
  }
}
