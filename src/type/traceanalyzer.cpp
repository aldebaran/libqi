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
    CallData* parent;
    CallData* asyncParent;
    std::list<CallData*> children; // child sync call sequences
    std::vector<CallData*> asyncChildren; // unordered
  };

  typedef std::list<CallData*> CallList;
  typedef boost::unordered_map<unsigned int, std::list<CallData*> > PerContext;
  typedef boost::unordered_map<unsigned int, CallData*> PerId;
  typedef boost::unordered_map<unsigned int, qi::EventTrace> TraceBuffer;

  class TraceAnalyzerImpl
  {
  public:
    PerContext perContext;
    PerId perId;
    TraceBuffer traceBuffer;
  };

  // Helpers for std algorithm on struct fields
  struct CompUid
  {
    CompUid() : v(0) {};
    CompUid(unsigned int v) : v(v) {}
    bool operator()(CallData* d)
    {
      return d->uid == v;
    }
    bool operator()(CallData * d, unsigned int uid)
    {
      return d->uid < uid;
    }
    unsigned int v;
  };
  struct CompEndTime
  {
    bool operator()(qi::int64_t t, CallData* d)
    {
      return t < d->tEnd;
    }
  };

  struct CallTime
  {
    CallTime(CallData * d) : t(d->tStart) {}
    CallTime(qi::int64_t t) : t(t) {}
    qi::int64_t t;
  };

  inline bool operator <(const CallTime& ct, CallData* cd)
  {
    return ct.t < cd->tStart;
  }

  inline qi::int64_t fromTV(const qi::os::timeval &tv)
  {
    return tv.tv_sec*1000000LL + tv.tv_usec;
  }

  // container helpers
  // delete container of pointers, swapping them out first
  template<typename T> void delete_content(T& v)
  {
    T copy;
    std::swap(copy, v);
    for (typename T::iterator it = copy.begin(), iend = copy.end(); it != iend; ++it)
      delete *it;
  }

  // delete range
  template<typename T> void delete_range(T& v, typename T::iterator start, typename T::iterator end)
  {
    T copy;
    copy.splice(copy.begin(), v, start, end);
    for (typename T::iterator it = copy.begin(), iend = copy.end(); it != iend; ++it)
      delete *it;
  }

  CallData::~CallData()
  {
    // unhook from parent
    if (parent)
      parent->children.remove(this); // might do nothing if we are in parent dtor
    // delete sync children
    delete_content(this->children);
    // disconnect async children
    for (unsigned i=0; i<asyncChildren.size(); ++i)
      asyncChildren[i]->asyncParent = 0;
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
    , parent(0)
    , asyncParent(0)
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
  static void insertTrace(CallList& l, CallData* d, CallData* parent = 0)
  {
    qiLogDebug() << "insertTrace " << d->uid;
    // Get first entry that started after us.
    CallList::iterator it = std::upper_bound(l.begin(), l.end(), CallTime(d));
    qiLogDebug() << "upperBoud " << ((it == l.end())? -1 : (int)(*it)->uid) << ' ' << (it == l.begin());
    if (it == l.begin())
    { // We are first
      l.insert(it, d);
      d->parent = parent;
    }
    else
    {
      // try to steal async children from entry above us
      CallList::iterator iprev = it;
      --iprev;
      CallData* prev = *iprev;
      for (unsigned i=0; i<prev->asyncChildren.size(); ++i)
      {
        CallData* child = prev->asyncChildren[i];
        if (child->tPost > d->tStart)
        { // This child was misplaced and is ours
          d->asyncChildren.push_back(child);
          prev->asyncChildren[i] = prev->asyncChildren.back();
          prev->asyncChildren.pop_back();
          child->asyncParent = d;
        }
      }
      // Check if we are child of it
      assert(prev->tStart <= d->tStart);
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
  static bool insertAsyncParentTrace(CallList& l, CallData* d)
  {
    if (l.empty())
    {
      qiLogDebug() << "empty";
      return false;
    }
    qiLogDebug() << l.front()->tStart;
    // Get first entry that started after our post time.
    CallList::iterator it = std::upper_bound(l.begin(), l.end(), CallTime(d->tPost));
    if (it == l.begin())
    { // damm, first element is older than us
      qiLogInfo() << "No async parent can be found";
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
    qiLogDebug() << "addTrace " << trace.id() << " " << trace.callerContext() << ' ' << trace.calleeContext() << ' ' << trace.kind();
    /* a trace-end event without the start available goes in
     * traceBuffer and that's it.
     * trace-end event with start available completes the entry
     * trace-start event gets inserted in the graph, and in the
     * per-uid perId map, so that future trace-end message can find it quickly
    */
    CallData* d = 0;
    CallList& lc = _p->perContext[trace.calleeContext()];
    EventTrace::EventKind k = trace.kind();
    if (k == EventTrace::Event_Call || k == EventTrace::Event_Signal)
    { // New element, insert in toplevel timeline or in trace children
      // Taking care of stealing async children if they were misplaced
      d = new CallData(obj, trace);
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
    CallList& container = d->parent?d->parent->children:lc;
    CallList::iterator it = std::find_if(container.begin(), container.end(), CompUid(trace.id()));
    if (it == lc.end())
    {
      qiLogInfo() << "Message not where it should be";
      return;
    }

    // now that we know when we stopped, eat-up children
    // well, that sentence did not came out as I expected
    CallList::iterator inext = it;
    ++inext;
    CallList::iterator iend = inext;
    while (iend != container.end() && (*iend)->tEnd && (*iend)->tEnd < (*it)->tEnd)
      ++iend;
    //CallList::iterator iend = std::upper_bound(inext, container.end(), (*it)->tEnd, CompEndTime());
    if (iend != inext)
      qiLogDebug() << "Splicing at least " << (*inext)->uid << ' ' << (*it)->tEnd << ' ' << (*inext)->tEnd;
    (*it)->children.splice((*it)->children.begin(), container, inext, iend);
    for (CallList::iterator ic = (*it)->children.begin(); ic != (*it)->children.end(); ++ic)
      (*ic)->parent = *it;

    // and also check if we are child of the element above
    if ((*it)->tEnd && it != container.begin())
    {
      CallList::iterator prev = it;
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
    for (PerContext::iterator it = _p->perContext.begin(), iend = _p->perContext.end();
      it != iend; ++it)
      delete_content(it->second);
    _p->perContext.clear();
    _p->perId.clear();
  }

  /* cleanup trace data for entry started before limit, includes children
  * Can break async links, but leaves the structure in a consistant state
  */
  void TraceAnalyzer::clear(const qi::os::timeval& tv)
  {
    qi::int64_t limit = fromTV(tv);
    for (PerContext::iterator it = _p->perContext.begin(), iend = _p->perContext.end();
      it != iend; ++it)
    {
      CallList& l = it->second;
      CallList::iterator it2 = std::upper_bound(l.begin(), l.end(), CallTime(limit));
      delete_range(l, l.begin(), it2);
    }
  }

  // cheap-o-tronic flow detection

  typedef TraceAnalyzer::FlowLink FlowLink;
  void trackLink(std::set<FlowLink>& links, const CallData* d)
  {
    for (CallList::const_iterator it = d->children.begin(), iend = d->children.end(); it != iend; ++it)
    {
      links.insert(FlowLink(d->obj, d->fun, (*it)->obj, (*it)->fun, true));
      trackLink(links, *it);
    }
    for (std::vector<CallData*>::const_iterator it = d->asyncChildren.begin(), iend = d->asyncChildren.end(); it != iend; ++it)
    {
      links.insert(FlowLink(d->obj, d->fun, (*it)->obj, (*it)->fun, false));
      // no need to recurse on async children
    }
  }

  void TraceAnalyzer::analyze(std::set<FlowLink>& links)
  {
    qiLogDebug() << "analyze";
    // First update missing async-parent data, due to unlucky event ordering
    for (PerContext::const_iterator it = _p->perContext.begin(), iend = _p->perContext.end();
      it != iend; ++it)
    {
      for (CallList::const_iterator it2 = it->second.begin(), iend2 = it->second.end(); it2 != iend2; ++it2)
      {
        // no need to recurse, you can't have both a sync and an async parent
        if (!(*it2)->asyncParent && (*it2)->tPost)
        {
          qiLogDebug() << "searching async parent " << (*it2)->uid;
          insertAsyncParentTrace(_p->perContext[(*it2)->callerCtx], *it2);
        }
      }
    }
    for (PerContext::iterator it = _p->perContext.begin(), iend = _p->perContext.end();
      it != iend; ++it)
    {
      for (CallList::iterator it2= it->second.begin(), it2end = it->second.end(); it2!=it2end; ++it2)
        trackLink(links, *it2);
    }
  }

  static void dumpTraces(std::ostream& o, const CallList& l, unsigned indent)
  {
    for (CallList::const_iterator it = l.begin(), iend = l.end(); it!=iend; ++it)
    {
      const CallData &cd = **it;
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
        for (unsigned i=0; i<cd.asyncChildren.size(); ++i)
        {
          o << cd.asyncChildren[i]->uid << ',';
        }
        o << '}';
      }
    }
  }

  void TraceAnalyzer::dumpTraces(std::ostream& o)
  {
    for (PerContext::const_iterator it = _p->perContext.begin(), iend = _p->perContext.end();
      it != iend; ++it)
    {
      o << it->first;
      qi::dumpTraces(o, it->second, 0);
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
