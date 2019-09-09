/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#include <boost/unordered_map.hpp>

#include <qi/log.hpp>

#include <qi/type/detail/traceanalyzer.hpp>

qiLogCategory("qitype.traceanalyzer");

namespace qi
{
  struct CallData
  {
    CallData(unsigned int obj, const EventTrace& et);
    ~CallData();

    void complete(const EventTrace& et);
    unsigned int uid;
    unsigned int obj;
    unsigned int fun;
    unsigned int ctx;
    unsigned int callerCtx;
    qi::int64_t tPost; // time at which call was posted() from parent
    qi::int64_t tStart;
    qi::int64_t tEnd; // 0 means not ended yet
    std::weak_ptr<CallData> parent;
    std::weak_ptr<CallData> asyncParent;
    std::list<std::shared_ptr<CallData>> children; // child sync call sequences
    std::vector<std::shared_ptr<CallData>> asyncChildren; // unordered
  };

  using CallList = std::list<std::shared_ptr<CallData>>;
  using PerContext = boost::unordered_map<unsigned int, std::list<std::shared_ptr<CallData>>>;
  using PerId = boost::unordered_map<unsigned int, std::shared_ptr<CallData>>;
  using TraceBuffer = boost::unordered_map<unsigned int, qi::EventTrace>;

  class TraceAnalyzerImpl
  {
  public:
    PerContext perContext;
    PerId perId;
    TraceBuffer traceBuffer;
  };

  struct CompareCallTime
  {
    CompareCallTime(CallData& d) : t(d.tStart) {}
    CompareCallTime(qi::int64_t t) : t(t) {}
    qi::int64_t t;

    friend bool operator<(const CompareCallTime& ct, std::shared_ptr<CallData> cd)
    {
      QI_ASSERT_TRUE(cd);
      return ct.t < cd->tStart;
    }

    friend bool operator<(std::shared_ptr<CallData> cd, const CompareCallTime& ct)
    {
      QI_ASSERT_TRUE(cd);
      return cd->tStart < ct.t;
    }
  };

  inline qi::int64_t fromTV(const qi::os::timeval &tv)
  {
    return tv.tv_sec*1000000LL + tv.tv_usec;
  }

  CallData::~CallData()
  {
    // unhook from parent
    if (auto lockedParent = parent.lock())
    {
      lockedParent->children.remove_if([=](const std::shared_ptr<CallData>& child){
        return child.get() == this;
      }); // might do nothing if we are in parent dtor
    }

    // clear sync children
    children.clear();

    // disconnect async children
    for (auto asyncChild : asyncChildren)
      if (asyncChild)
        asyncChild->asyncParent.reset();
  }

  TraceAnalyzer::TraceAnalyzer()
  : _p(new TraceAnalyzerImpl())
  {}

  TraceAnalyzer::~TraceAnalyzer()
  {}


  CallData::CallData(unsigned int obj, const EventTrace& et)
    : uid(et.id())
    , obj(obj)
    , fun(et.slotId())
    , ctx(et.calleeContext())
    , callerCtx(et.callerContext())
    , tPost(fromTV(et.postTimestamp()))
    , tStart(fromTV(et.timestamp()))
    , tEnd(0)
    {}

  void CallData::complete(const EventTrace& et)
  {
    tEnd = fromTV(et.timestamp());
  }
  /* Events can reach us in basically random orders.
  * Synchronous hierarchy is constructed when an event arrives (it places
  * itself as deep as it can, but only goes as a child if it is sures
  * the decision is correct), and when an event is completed (end time
  * received, then it recheck following events to check if they were
  * actually children of his).
  *
  * Async hierarchy is double-checked the same way: When a new event inserts
  * itself, it tries to steal async children from its parents. And it
  * inserts itself as async child of the best correct match
  *
  *
  * Assumes end event is reached after start event for same id
  */
  static void insertTrace(CallList& l,
                          const std::shared_ptr<CallData>& d,
                          const std::weak_ptr<CallData>& parent = {})
  {
    qiLogDebug() << "insertTrace " << d->uid;

    // Get first entry that started after us.
    const auto it = std::upper_bound(l.begin(), l.end(), CompareCallTime(*d));
    qiLogDebug() << "upperBoud " << ((it == l.end()) ? -1 : static_cast<int>((*it)->uid)) << ' '
                 << (it == l.begin());

    if (it == l.begin())
    { // We are first
      l.insert(it, d);
      d->parent = parent;
    }
    else
    {
      // try to steal async children from entry above us
      auto iprev = it;
      --iprev;
      const auto prev = *iprev;
      for (auto& asyncChild : prev->asyncChildren)
      {
        auto asyncChildCpy = asyncChild;
        if (asyncChildCpy->tPost > d->tStart)
        { // This child was misplaced and is ours
          d->asyncChildren.push_back(asyncChildCpy);
          asyncChild = prev->asyncChildren.back();
          prev->asyncChildren.pop_back();
          asyncChildCpy->asyncParent = d;
        }
      }
      // Check if we are child of it
      QI_ASSERT(prev->tStart <= d->tStart);
      if (prev->tEnd >= d->tStart)
      {
        qiLogDebug() << "Insert to child " << d->tStart << ' ' << d->tEnd
         << ' ' << prev->tStart << ' ' << prev->tEnd;
        insertTrace(prev->children, d, prev); // we start within it interval: we are its chilrend
      }
      else
      {
        l.insert(it, d);
        d->parent = parent;
      }
    }
    // if "it" was not completed, it will check following elements again
    // when tEnd will be available
  }

  // Find out our async parent and add to it
  static bool insertAsyncParentTrace(CallList& l, const std::shared_ptr<CallData>& d)
  {
    if (l.empty())
    {
      qiLogDebug() << "empty";
      return false;
    }
    qiLogDebug() << l.front()->tStart;
    // Get first entry that started after our post time.
    auto it = std::upper_bound(l.begin(), l.end(), CompareCallTime(d->tPost));
    if (it == l.begin())
    { // damm, first element is older than us
      qiLogVerbose() << "No async parent can be found";
      return false;
    }
    --it;
    qiLogDebug() << "Child check";
    // try if a sync child is better placed than it
    bool wasInserted = insertAsyncParentTrace((*it)->children, d);
    if (wasInserted)
      return true;
    // was not inserted in children, insert here
    (*it)->asyncChildren.push_back(d);
    d->asyncParent = *it;
    return true;
  }


  // handle a new EventTrace
  void TraceAnalyzer::addTrace(const qi::EventTrace& trace, unsigned int obj)
  {
    qiLogDebug() << "addTrace " << trace.id() << " " << trace.callerContext() << ' '
                 << trace.calleeContext() << ' ' << trace.kind();
    /* a trace-end event without the start available goes in
     * traceBuffer and that's it.
     * trace-end event with start available completes the entry
     * trace-start event gets inserted in the graph, and in the
     * per-uid perId map, so that future trace-end message can find it quickly
    */
    std::shared_ptr<CallData> d;
    CallList& lc = _p->perContext[trace.calleeContext()];
    EventTrace::EventKind k = trace.kind();
    if (k == EventTrace::Event_Call || k == EventTrace::Event_Signal)
    { // New element, insert in toplevel timeline or in trace children
      // Taking care of stealing async children if they were misplaced
      d = std::make_shared<CallData>(obj, trace);
      insertTrace(lc, d);
      // check if we have an async parent
      if (d->tPost)
      { // if tPost is set, we have an async parent.
        // Note that it can be on the same context as us
        qiLogDebug() << "look for async parent " << trace.callerContext();
        CallList& asyncParentCtx = _p->perContext[trace.callerContext()];
        insertAsyncParentTrace(asyncParentCtx, d);
      }
      TraceBuffer::iterator it = _p->traceBuffer.find(trace.id());
      if (it != _p->traceBuffer.end())
      { // end already there
        d->complete(it->second);
        _p->traceBuffer.erase(it);
        // ...and go on below
      }
      else
      {
        _p->perId[d->uid] = d;
        return;
      }
    }
    else
    {
      // Complete the element.
      d = _p->perId[trace.id()];
      if (!d)
      {
        // We got the reply before the call, store it
        _p->traceBuffer[trace.id()] = trace;
        //qiLogWarning() << "Trace not registered : " << trace.id();
        return;
      }
      d->complete(trace);
      _p->perId.erase(trace.id());
    }
    // look for the list that should contain us
    CallList& container = [&]() -> CallList& {
      if (auto parentLock = d->parent.lock())
        return parentLock->children;
      return lc;
    }();
    const auto it =
        std::find_if(container.begin(), container.end(),
                     [&](const std::shared_ptr<CallData>& d) { return d && d->uid == trace.id(); });
    if (it == container.end())
    {
      qiLogVerbose() << "Message not where it should be";
      return;
    }

    // now that we know when we stopped, eat-up children
    // well, that sentence did not came out as I expected
    auto inext = it;
    ++inext;
    auto iend = inext;
    while (iend != container.end() && (*iend)->tEnd && (*iend)->tEnd < (*it)->tEnd)
      ++iend;
    if (iend != inext)
      qiLogDebug() << "Splicing at least " << (*inext)->uid << ' ' << (*it)->tEnd << ' ' << (*inext)->tEnd;
    (*it)->children.splice((*it)->children.begin(), container, inext, iend);
    for (auto& child : (*it)->children)
      child->parent = *it;

    // and also check if we are child of the element above
    if ((*it)->tEnd && it != container.begin())
    {
      auto prev = it;
      --prev;
      if ((*prev)->tEnd && (*prev)->tEnd > (*it)->tEnd)
      {
        qiLogDebug() << "Append to prev's children "
          << (*prev)->tEnd << ' ' << (*it)->tEnd << ' '
          << (*prev)->tStart << ' ' << (*it)->tStart;
        insertTrace((*prev)->children, *it, *prev);
        container.erase(it);
      }
    }
  }

  void TraceAnalyzer::clear()
  {
    _p->perContext.clear();
    _p->perId.clear();
  }

  /* cleanup trace data for entry started before limit, includes children
  * Can break async links, but leaves the structure in a consistant state
  */
  void TraceAnalyzer::clear(const qi::os::timeval& tv)
  {
    qi::int64_t limit = fromTV(tv);
    for (auto& perContextPair : _p->perContext)
    {
      CallList& l = perContextPair.second;
      l.erase(l.begin(), std::upper_bound(l.begin(), l.end(), CompareCallTime(limit)));
    }
  }

  // cheap-o-tronic flow detection

  using FlowLink = TraceAnalyzer::FlowLink;
  void trackLink(std::set<FlowLink>& links, const std::shared_ptr<const CallData>& d)
  {
    for (const auto& child : d->children)
    {
      links.insert(FlowLink(d->obj, d->fun, child->obj, child->fun, true));
      trackLink(links, child);
    }
    for (const auto& asyncChild : d->asyncChildren)
    {
      links.insert(FlowLink(d->obj, d->fun, asyncChild->obj, asyncChild->fun, false));
      // no need to recurse on async children
    }
  }

  void TraceAnalyzer::analyze(std::set<FlowLink>& links)
  {
    qiLogDebug() << "analyze";
    // First update missing async-parent data, due to unlucky event ordering
    for (const auto& perContextPair : _p->perContext)
    {
      const auto& dataList = perContextPair.second;
      for (const auto& data : dataList)
      {
        // no need to recurse, you can't have both a sync and an async parent
        if (data->asyncParent.expired() && data->tPost)
        {
          qiLogDebug() << "searching async parent " << data->uid;
          insertAsyncParentTrace(_p->perContext[data->callerCtx], data);
        }
      }
    }
    for (const auto& perContextPair : _p->perContext)
    {
      const auto& dataList = perContextPair.second;
      for (const auto& data : dataList)
        trackLink(links, data);
    }
  }

  static void dumpTraces(std::ostream& o, const CallList& l, unsigned indent)
  {
    for (const auto& call : l)
    {
      const CallData &cd = *call;
      o << ' ' << cd.uid << ':' << cd.obj << '.' << cd.fun;
      if (cd.children.size())
      {
        o << '<';
        dumpTraces(o, cd.children, indent+1);
        o << '>';
      }
      if (cd.asyncChildren.size())
      {
        o << '{';
        // just ref async children
        for (auto& asyncChild : cd.asyncChildren)
        {
          o << asyncChild->uid << ',';
        }
        o << '}';
      }
    }
  }

  void TraceAnalyzer::dumpTraces(std::ostream& o)
  {
    for (const auto& perContextPair : _p->perContext)
    {
      o << perContextPair.first;
      qi::dumpTraces(o, perContextPair.second, 0);
      o << std::endl;
    }

  }
  std::string TraceAnalyzer::dumpTraces()
  {
    std::stringstream s;
    dumpTraces(s);
    return s.str();
  }
}
