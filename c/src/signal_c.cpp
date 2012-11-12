/*
** Author(s):
**  - Julien Freche <jfreche@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2011, 2012 Aldebararan Robotics
*/

#include <qitype/signal.hpp>

#include <qimessaging/c/signal_c.h>
#include "signal_c_p.h"

qi_signal_t* qi_signal_create()
{
  qi::Signal<void (void*)>* sig = new qi::Signal<void (void*)>();

  return (qi_signal_t*)sig;
}

void qi_signal_destroy(qi_signal_t* sig)
{
  qi::Signal<void (void*)>* signal = reinterpret_cast<qi::Signal<void (void*)>* >(sig);

  delete signal;
}

unsigned int qi_signal_connect(qi_signal_t* sig, qi_signal_callback_t f)
{
  qi::Signal<void (void*)>* signal = reinterpret_cast<qi::Signal<void (void*)>* >(sig);
  qi::SignalBase::Link l = signal->connect(f);

  return l;
}

bool qi_signal_disconnect(qi_signal_t* sig, unsigned int l)
{
  qi::Signal<void (void*)>* signal = reinterpret_cast<qi::Signal<void (void*)>* >(sig);

  return signal->disconnect(l);
}

bool qi_signal_disconnect_all(qi_signal_t* sig)
{
  qi::Signal<void (void*)>* signal = reinterpret_cast<qi::Signal<void (void*)>* >(sig);

  return signal->disconnectAll();
}

void qi_signal_trigger(qi_signal_t* sig, void* param)
{
  qi::Signal<void (void*)>* signal = reinterpret_cast<qi::Signal<void (void*)>* >(sig);

  (*signal)(param);
}
