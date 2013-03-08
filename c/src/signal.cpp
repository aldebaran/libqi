/*
** Author(s):
**  - Julien Freche <jfreche@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2011, 2012 Aldebararan Robotics
*/

#include <qitype/signal.hpp>
#include <qic/signal.h>


qi_signal_t* qi_signal_create()
{
  qi::Signal<void (qi::GenericValue)>* sig = new qi::Signal<void (qi::GenericValue)>();

  return (qi_signal_t*)sig;
}

void qi_signal_destroy(qi_signal_t* sig)
{
  qi::Signal<void (qi::GenericValue)>* signal = reinterpret_cast<qi::Signal<void (qi::GenericValue)>* >(sig);

  delete signal;
}

unsigned int qi_signal_connect(qi_signal_t* sig, qi_signal_callback_t f, void *data)
{
  qi::Signal<void (qi::GenericValue)>* signal = reinterpret_cast<qi::Signal<void (qi::GenericValue)>* >(sig);
  qi::SignalBase::Link l;// = signal->connect(f);

  return l;
}

bool qi_signal_disconnect(qi_signal_t* sig, unsigned int l)
{
  qi::Signal<void (qi::GenericValue)>* signal = reinterpret_cast<qi::Signal<void (qi::GenericValue)>* >(sig);

  return signal->disconnect(l);
}

bool qi_signal_disconnect_all(qi_signal_t* sig)
{
  qi::Signal<void (qi::GenericValue)>* signal = reinterpret_cast<qi::Signal<void (qi::GenericValue)>* >(sig);

  return signal->disconnectAll();
}

//TODO: (signal, value[]) ?
void qi_signal_trigger(qi_signal_t* sig, qi_value_t* param)
{
  qi::Signal<void (qi::GenericValue)>* signal = reinterpret_cast<qi::Signal<void (qi::GenericValue)>* >(sig);

  //(*signal)(param);
}
