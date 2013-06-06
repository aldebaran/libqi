/*
**
** Author(s):
**  - Pierre ROULLON <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <qi/log.hpp>
#include <qitype/signature.hpp>
#include <qitype/genericobjectbuilder.hpp>
#include <qitype/genericobject.hpp>
#include <qitype/genericvalue.hpp>
#include <qitype/typedispatcher.hpp>
#include <qitype/type.hpp>

#include <jnitools.hpp>
#include <jobjectconverter.hpp>
#include <map_jni.hpp>
#include <list_jni.hpp>
#include <tuple_jni.hpp>
#include <object_jni.hpp>

qiLogCategory("qimessaging.jni");
using namespace qi;

struct toJObject
{
    toJObject(jobject *result)
      : result(result), jni_version(QI_JNI_MIN_VERSION)
    {
      if (JVM()->GetEnv((void **) &env, jni_version) != JNI_OK)
        qiLogFatal("qimessaging.jni") << "Cannot initialize Java environment.";
    }

    void visitUnknown(qi::GenericValuePtr value)
    {
      throwJavaError(env, "Error in conversion: Unable to convert unknown type in Java");
    }

    void visitInt(qi::int64_t value, bool isSigned, int byteSize)
    {
      // Clear all remaining exceptions
      env->ExceptionClear();

      // Get Integer class template
      // ... or Boolean if byteSize is 0
      jclass cls = qi::jni::clazz(byteSize == 0 ? "Boolean" : "Integer");
      if (env->ExceptionCheck())
      {
        qi::jni::releaseClazz(cls);
        throwJavaError(env, "GenericValue to Integer : FindClass error");
        return;
      }

      // Find constructor method ID
      jmethodID mid = env->GetMethodID(cls, "<init>", byteSize == 0 ? "(Z)V" : "(I)V");
      if (!mid)
      {
        qi::jni::releaseClazz(cls);
        throwJavaError(env, "GenericValue to Integer : GetMethodID error");
        return;
      }

      // Instanciate new Integer, yeah !
      jint jval = value;
      *result = env->NewObject(cls, mid, jval);
      checkForError();
      qi::jni::releaseClazz(cls);
    }

    void visitString(char *data, size_t QI_UNUSED(len))
    {
      if (data)
        *result = (jobject) env->NewStringUTF(data);
      else
        *result = (jobject) env->NewStringUTF("");
      checkForError();
    }

    void visitVoid()
    {
      jclass cls = env->FindClass("java/lang/Void");
      jmethodID mid = env->GetMethodID(cls, "<init>", "()V");
      *result = env->NewObject(cls, mid);
      checkForError();
    }

    void visitFloat(double value, int byteSize)
    {
      // Clear all remaining exceptions
      env->ExceptionClear();

      // Get Float class template
      jclass cls = qi::jni::clazz("Float");
      if (env->ExceptionCheck())
      {
        qi::jni::releaseClazz(cls);
        throwJavaError(env, "GenericValue to Float : FindClass error");
        return;
      }

      // Find constructor method ID
      jmethodID mid = env->GetMethodID(cls, "<init>","(F)V");
      if (!mid)
      {
        qi::jni::releaseClazz(cls);
        throwJavaError(env, "GenericValue to Float : GetMethodID error");
        return;
      }

      // Instanciate new Float, yeah !
      jfloat jval = value;
      *result = env->NewObject(cls, mid, jval);
      qi::jni::releaseClazz(cls);
      checkForError();
    }

    void visitList(qi::GenericIterator it, qi::GenericIterator end)
    {
      JNIList list; // this is OK.

      for(; it != end; ++it)
      {
        jobject current = (*it).to<jobject>();
        list.push_back(current);
      }

      it.destroy();
      end.destroy();
      *result = list.object();
    }

    void visitMap(qi::GenericIterator it, qi::GenericIterator end)
    {
      jobject key, value;
      JNIHashTable ht;

      for (; it != end; ++it)
      {
        key = JObject_from_GenericValue((*it)[0]);
        value = JObject_from_GenericValue((*it)[1]);

        ht.setItem(key, value);
      }

      *result = ht.object();
      it.destroy();
      end.destroy();
    }

    void visitObject(qi::GenericObject obj)
    {
      throw std::runtime_error("Cannot convert GenericObject to Jobject.");
    }

    void visitObjectPtr(qi::ObjectPtr o)
    {

      try
      {
        JNIObject obj(o);
        *result = obj.object();

      } catch (std::exception&)
      {
        return;
      }

    }

    void visitPointer(qi::GenericValuePtr pointee)
    {
      qiLogFatal() << "Error in conversion: Unable to convert pointer in Java";
      throwJavaError(env, "Error in conversion: Unable to convert pointer in Java");
    }

    void visitTuple(const std::string& className, const std::vector<qi::GenericValuePtr>& tuple, const std::vector<std::string>& annotations)
    {
      JNITuple jtuple(tuple.size());
      int i = 0;

      for(std::vector<qi::GenericValuePtr>::const_iterator it = tuple.begin(); it != tuple.end(); ++it)
      {
        jobject current = (*it).to<jobject>();
        jtuple.set(i++, current);
      }

      *result = jtuple.object();
    }

    void visitDynamic(qi::GenericValuePtr pointee)
    {
      *result = JObject_from_GenericValue(pointee);
    }

    void visitRaw(qi::GenericValuePtr value)
    {
      // Not tested.
      /* Encapuslate the buffer in ByteBuffer */
      qi::Buffer buf = value.as<qi::Buffer>();

      // Create a new ByteBuffer and reserve enough space
      jclass cls = env->FindClass("java/lang/ByteBuffer");
      jmethodID mid = env->GetMethodID(cls, "init","(I)V");
      jobject ar = env->NewObject(cls, mid, buf.size() + 1);


      // Put qi::Buffer content into ByteByffer
      mid = env->GetMethodID(cls, "put","([B]II)[LJava/lang/ByteBuffer;");
      *result = env->CallObjectMethod(ar, mid, buf.data(), 0, buf.size());

      checkForError();
      env->DeleteLocalRef(cls);
      env->DeleteLocalRef(ar);
    }

    void visitIterator(qi::GenericValuePtr v)
    {
      visitUnknown(v);
    }

    void checkForError()
    {
      if (result == NULL)
        throwJavaError(env, "Error in conversion to JObject");
    }

    jobject* result;
    int      jni_version;
    JNIEnv*  env;

}; // !toJObject

jobject JObject_from_GenericValue(qi::GenericValuePtr val)
{
  jobject result= NULL;
  toJObject tjo(&result);
  qi::typeDispatch<toJObject>(tjo, val);
  return result;
}

void JObject_from_GenericValue(qi::GenericValuePtr val, jobject* target)
{
  toJObject tal(target);
  qi::typeDispatch<toJObject>(tal, val);
}

qi::GenericValuePtr GenericValue_from_JObject_List(jobject val)
{
  JNIEnv* env;
  JNIList list(val);
  std::vector<qi::GenericValue>& res = *new std::vector<qi::GenericValue>();
  int size = 0;

  JVM()->GetEnv((void **) &env, QI_JNI_MIN_VERSION);

  size = list.size();
  res.reserve(size);
  for (int i = 0; i < size; i++)
  {
    jobject current = list.get(i);
    res.push_back(qi::GenericValue(GenericValue_from_JObject(current).first));
  }

  return qi::GenericValueRef(res);
}

qi::GenericValuePtr GenericValue_from_JObject_Map(jobject hashtable)
{
  JNIEnv* env;
  std::map<qi::GenericValue, qi::GenericValue>& res = *new std::map<qi::GenericValue, qi::GenericValue>();
  JNIHashTable ht(hashtable);
  jobject key, value;

  JVM()->GetEnv((void **) &env, QI_JNI_MIN_VERSION);

  JNIEnumeration keys = ht.keys();
  while (keys.hasNextElement())
  {
    key = keys.nextElement();
    value = ht.at(key);
    qi::GenericValuePtr newKey = GenericValue_from_JObject(key).first;
    qi::GenericValuePtr newValue = GenericValue_from_JObject(value).first;
    res[qi::GenericValue(newKey)] = newValue;
    env->DeleteLocalRef(key);
    env->DeleteLocalRef(value);
  }
  return qi::GenericValueRef(res);
}

qi::GenericValuePtr GenericValue_from_JObject_Tuple(jobject val)
{
  JNITuple tuple(val);
  int i = 0;
  std::vector<qi::GenericValuePtr>& res = *new std::vector<qi::GenericValuePtr>();

  while (i < tuple.size())
  {
    qi::GenericValuePtr value = GenericValue_from_JObject(tuple.get(i)).first;
    res.push_back(value);
    i++;
  }

  return qi::makeGenericTuple(res);
}

qi::GenericValuePtr GenericValue_from_JObject_RemoteObject(jobject val)
{
  JNIObject obj(val);

  qi::ObjectPtr* tmp = new qi::ObjectPtr();
  *tmp = obj.objectPtr();
  return qi::GenericValueRef(*tmp);
}

std::pair<qi::GenericValuePtr, bool> GenericValue_from_JObject(jobject val)
{
  qi::GenericValuePtr res;
  JNIEnv* env;
  bool copy = false;

  if (!val)
    throw std::runtime_error("Unable to convert JObject in GenericValue (Value is null)");

  if (JVM()->GetEnv((void **) &env, QI_JNI_MIN_VERSION) != JNI_OK)
    throw std::runtime_error("No JNIEnvironment available for conversion.");

  if (JVM()->AttachCurrentThread((envPtr) &env, (void *) 0) != JNI_OK)
    throw std::runtime_error("Cannot attach current thread to JVM for conversion.");

  jclass stringClass = qi::jni::clazz("String");
  jclass int32Class = qi::jni::clazz("Integer");
  jclass floatClass = qi::jni::clazz("Float");
  jclass doubleClass = qi::jni::clazz("Double");
  jclass boolClass = qi::jni::clazz("Boolean");
  jclass longClass = qi::jni::clazz("Long");
  jclass mapClass = qi::jni::clazz("Map");
  jclass listClass = qi::jni::clazz("List");
  jclass tupleClass = qi::jni::clazz("Tuple");
  jclass objectClass = qi::jni::clazz("Object");

  if (val == NULL)
  {
    res = qi::GenericValuePtr(qi::typeOf<void>());
  }
  else if (env->IsInstanceOf(val, stringClass))
  {
    const char* data = env->GetStringUTFChars((jstring) val, 0);
    std::string tmp = std::string(data);
    env->ReleaseStringUTFChars((jstring) val, data);
    res = qi::GenericValueRef(*new std::string(tmp));
    copy = true;
  }
  else if (env->IsInstanceOf(val, floatClass))
  {
    jmethodID mid = env->GetMethodID(floatClass, "floatValue","()F");
    jfloat v = env->CallFloatMethod(val, mid);
    res = qi::GenericValueRef((float)v).clone();
    copy = true;
  }
  else if (env->IsInstanceOf(val, doubleClass)) // If double, convert to float
  {
    jmethodID mid = env->GetMethodID(doubleClass, "doubleValue","()D");
    jfloat v = (jfloat) env->CallDoubleMethod(val, mid);
    res = qi::GenericValueRef((float)v).clone();
    copy = true;
  }
  else if (env->IsInstanceOf(val, longClass))
  {
    jmethodID mid = env->GetMethodID(longClass, "longValue","()L");
    jlong v = env->CallLongMethod(val, mid);
    res = qi::GenericValueRef(v).clone();
    copy = true;
  }
  else if (env->IsInstanceOf(val, boolClass))
  {
    jmethodID mid = env->GetMethodID(boolClass, "booleanValue","()Z");
    jboolean v = env->CallBooleanMethod(val, mid);
    res = qi::GenericValueRef((bool) v).clone();
    copy = true;
  }
  else if (env->IsInstanceOf(val, int32Class))
  {
    jmethodID mid = env->GetMethodID(int32Class, "intValue","()I");
    jint v = env->CallIntMethod(val, mid);
    res = qi::GenericValueRef((int) v).clone();
    copy = true;
  }
  else if (env->IsInstanceOf(val, listClass))
  {
    copy = true;
    res = GenericValue_from_JObject_List(val);
  }
  else if (env->IsInstanceOf(val, mapClass))
  {
    copy = true;
    res = GenericValue_from_JObject_Map(val);
  }
  else if (env->IsAssignableFrom(env->GetObjectClass(val), tupleClass))
  {
    res = GenericValue_from_JObject_Tuple(val);
  }
  else if (env->IsInstanceOf(val, objectClass))
  {
    res = GenericValue_from_JObject_RemoteObject(val);
  }
  else
  {
    qiLogError() << "Cannot serialize return value: Unable to convert JObject in GenericValue";
    throw std::runtime_error("Cannot serialize return value: Unable to convert JObject in GenericValue");
  }


  qi::jni::releaseClazz(stringClass);
  qi::jni::releaseClazz(int32Class);
  qi::jni::releaseClazz(floatClass);
  qi::jni::releaseClazz(doubleClass);
  qi::jni::releaseClazz(boolClass);
  qi::jni::releaseClazz(longClass);
  qi::jni::releaseClazz(mapClass);
  qi::jni::releaseClazz(listClass);
  qi::jni::releaseClazz(tupleClass);
  qi::jni::releaseClazz(objectClass);

  return std::make_pair(res, copy);
}


/*
 * Define this struct to add jobject to the type system.
 * That way we can manipulate jobject transparently.
 * - We have to override clone and destroy here to be compliant
 *   with the java reference counting. Otherwise, the value could
 *   be Garbage Collected as we try to manipulate it.
 * - We register the type as 'jobject' since java methods manipulates
 *   objects only by this typedef pointer, never by value and we do not want to copy
 *   a jobject.
 */
class JObjectType: public qi::TypeDynamic
{
  public:

    virtual const qi::TypeInfo& info()
    {
      static qi::TypeInfo* result = 0;
      if (!result)
        result = new qi::TypeInfo(typeid(jobject));

      return *result;
    }

    virtual void* initializeStorage(void* ptr = 0)
    {
      // ptr is jobject* (aka _jobject**)
      return ptr;
    }

    virtual void* ptrFromStorage(void** s)
    {
      jobject* tmp = (jobject*) s;
      return *tmp;
    }

    virtual qi::GenericValuePtr get(void* storage)
    {
      return GenericValue_from_JObject(*((jobject*)ptrFromStorage(&storage))).first;
    }

    virtual void set(void** storage, qi::GenericValuePtr src)
    {
      // storage is jobject*
      // We will assign *storage to target, so we need it to be allocated
      jobject *target = new jobject;

      // Giving jobject* to JObject_from_GenericValue
      JObject_from_GenericValue(src, target);

      JNIEnv *env;
      JVM()->GetEnv((void **) &env, QI_JNI_MIN_VERSION);
      JVM()->AttachCurrentThread((envPtr) &env, (void *) 0);
      env->NewGlobalRef(*target);

      *storage = target;
    }

    virtual void* clone(void* obj)
    {
      jobject*    ginstance = (jobject*) obj;

      if (!obj)
        return 0;

      jobject* cloned = new jobject;
      *cloned = JObject_from_GenericValue(qi::GenericValueRef(*ginstance));

      return cloned;
    }

    virtual void destroy(void* obj)
    {
      // void* obj is a jobject
      jobject jobj = (jobject) obj;

      JNIEnv *env;
      JVM()->GetEnv((void **) &env, QI_JNI_MIN_VERSION);
      JVM()->AttachCurrentThread((envPtr) &env, (void *) 0);
      env->DeleteGlobalRef(jobj);
    }

    virtual bool less(void* a, void* b)
    {
      jobject* pa = (jobject*) ptrFromStorage(&a);
      jobject* pb = (jobject*) ptrFromStorage(&b);

      return *pa < *pb;
    }
};

/* Register jobject -> See the above comment for explanations */
QI_TYPE_REGISTER_CUSTOM(jobject, JObjectType);

