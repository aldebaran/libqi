#include <qi/type/detail/manageable.hpp>
#include <qi/type/objecttypebuilder.hpp>
#include "../type/signal_p.hpp"

namespace qi
{

  class ManageablePrivate
  {
  public:
    ManageablePrivate();
    ~ManageablePrivate();
    // SignalLinks that target us. Needed to be able to disconnect upon destruction
    std::vector<SignalSubscriber>       registrations;
    mutable boost::mutex                registrationsMutex;
    bool                                dying;
    // Event loop in which calls are made if set
    boost::shared_ptr<ExecutionContext> executionContext;
    boost::mutex                        initMutex;

    bool statsEnabled;
    bool traceEnabled;
    ObjectStatistics stats;
    qi::Atomic<int> traceId;
  };

  ManageablePrivate::ManageablePrivate()
    : dying(false)
    , statsEnabled(false)
    , traceEnabled(false)
  {
  }

  ManageablePrivate::~ManageablePrivate()
  {
    dying = true;
    std::vector<SignalSubscriber> copy;
    {
      boost::mutex::scoped_lock sl(registrationsMutex);
      copy = registrations;
    }
    for (unsigned i = 0; i < copy.size(); ++i)
    {
      if(auto source = copy[i]._p->source.lock())
        source->disconnect(copy[i]._p->linkId).wait();
    }
  }

  Manageable::Manageable()
    : traceObject([this](bool enable){ enableTrace(enable); return Future<void>{0}; })
    , _p(new ManageablePrivate())
  {
  }

  Manageable::Manageable(const Manageable& b)
    : Manageable()
  {
    _p->executionContext = b._p->executionContext;
  }

  Manageable& Manageable::operator=(const Manageable& b)
  {
    if (this == &b)
      return *this;

    _p.reset(new ManageablePrivate());
    _p->executionContext = b._p->executionContext;

    return *this;
  }

  Manageable::~Manageable()
  {
  }

  boost::mutex& Manageable::initMutex()
  {
    return _p->initMutex;
  }

  void Manageable::forceExecutionContext(
      boost::shared_ptr<ExecutionContext> ec)
  {
    _p->executionContext = ec;
  }

  boost::shared_ptr<ExecutionContext> Manageable::executionContext() const
  {
    return _p->executionContext;
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
    boost::mutex::scoped_lock l(_p->registrationsMutex);
    MethodStatistics& ms = _p->stats[slotId];
    ms.push(wallTime, userTime, systemTime);
  }

  ObjectStatistics Manageable::stats() const
  {
    boost::mutex::scoped_lock l(_p->registrationsMutex);
    return _p->stats;
  }

  void Manageable::clearStats()
  {
    boost::mutex::scoped_lock l(_p->registrationsMutex);
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
  static Manageable::MethodMap* methodMap = nullptr;
  static Manageable::SignalMap* signalMap = nullptr;
  static MetaObject* metaObject = nullptr;
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
    QI_ASSERT(id <= endId);
    const detail::ObjectTypeData& typeData = builder.typeData();
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
