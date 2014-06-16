#include <qi/type/details/manageable.hpp>
#include <qi/type/objecttypebuilder.hpp>

#include "staticobjecttype.hpp"
#include "anyobject_p.hpp"

namespace qi
{

ManageablePrivate::ManageablePrivate()
: objectMutex(new boost::recursive_timed_mutex)
, statsEnabled(false)
, traceEnabled(false)
{
}

Manageable::Manageable()
: traceObject(boost::bind(&Manageable::enableTrace, this, _1))
{
  _p = new ManageablePrivate();
  _p->eventLoop = 0;
  _p->dying = false;
}

Manageable::Manageable(const Manageable& b)
: traceObject(boost::bind(&Manageable::enableTrace, this, _1))
{
  _p = new ManageablePrivate();
  _p->eventLoop = b._p->eventLoop;
  _p->dying = false;
}

void Manageable::operator=(const Manageable& b)
{
  this->~Manageable();
  _p = new ManageablePrivate();
  _p->eventLoop = b._p->eventLoop;
  _p->dying = false;
}

Manageable::~Manageable()
{
  _p->dying = true;
  std::vector<SignalSubscriber> copy;
  {
    boost::mutex::scoped_lock sl(_p->registrationsMutex);
    copy = _p->registrations;
  }
  for (unsigned i = 0; i < copy.size(); ++i)
  {
    copy[i].source->disconnect(copy[i].linkId);
  }
  delete _p;
}

void Manageable::forceEventLoop(EventLoop* el)
{
  _p->eventLoop = el;
}

EventLoop* Manageable::eventLoop() const
{
  return _p->eventLoop;
}

Manageable::TimedMutexPtr Manageable::mutex()
{
  return _p->objectMutex;
}

bool Manageable::isStatsEnabled() const
{
  return _p->statsEnabled;
}

void Manageable::enableStats(bool state)
{
  _p->statsEnabled = state;
}

void Manageable::pushStats(int slotId, float wallTime, float userTime, float systemTime)
{
  boost::mutex::scoped_lock(_p->registrationsMutex);
  MethodStatistics& ms = _p->stats[slotId];
  ms.push(wallTime, userTime, systemTime);
}

ObjectStatistics Manageable::stats() const
{
  boost::mutex::scoped_lock(_p->registrationsMutex);
  return _p->stats;
}

void Manageable::clearStats()
{
  boost::mutex::scoped_lock(_p->registrationsMutex);
  _p->stats.clear();
}

bool Manageable::isTraceEnabled() const
{
  return _p->traceEnabled;
}

void Manageable::enableTrace(bool state)
{
  _p->traceEnabled = state;
}

int Manageable::_nextTraceId()
{
  return ++_p->traceId;
}

namespace manageable
{
  static Manageable::MethodMap* methodMap = 0;
  static Manageable::SignalMap* signalMap = 0;
  static MetaObject* metaObject = 0;
}
void Manageable::_build()
{
  if (manageable::methodMap)
    return;
  manageable::methodMap = new MethodMap();
  manageable::signalMap = new SignalMap();
  manageable::metaObject = new MetaObject();
  ObjectTypeBuilder<Manageable> builder;
  unsigned int id = startId;
  builder.advertiseMethod("isStatsEnabled", &Manageable::isStatsEnabled, MetaCallType_Auto, id++);
  builder.advertiseMethod("enableStats", &Manageable::enableStats,       MetaCallType_Auto, id++);
  builder.advertiseMethod("stats", &Manageable::stats,                   MetaCallType_Auto, id++);
  builder.advertiseMethod("clearStats", &Manageable::clearStats,         MetaCallType_Auto, id++);
  builder.advertiseMethod("isTraceEnabled", &Manageable::isTraceEnabled, MetaCallType_Auto, id++);
  builder.advertiseMethod("enableTrace", &Manageable::enableTrace,       MetaCallType_Auto, id++);
  builder.advertiseSignal("traceObject", &Manageable::traceObject, id++);
  assert(id <= endId);
  const ObjectTypeData& typeData = builder.typeData();
  *manageable::methodMap = typeData.methodMap;
  *manageable::signalMap = typeData.signalGetterMap;
  *manageable::metaObject = builder.metaObject();
}

Manageable::MethodMap& Manageable::manageableMmethodMap()
{
  _build();
  return *manageable::methodMap;
}

Manageable::SignalMap& Manageable::manageableSignalMap()
{
  _build();
  return *manageable::signalMap;
}

MetaObject& Manageable::manageableMetaObject()
{
  _build();
  return *manageable::metaObject;
}

}
