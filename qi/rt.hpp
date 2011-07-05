#ifndef RT_HPP
#define RT_HPP

#include <ctime>

namespace qi {
namespace rt {


/** \brief create an rt thread
  * throw qi::os_error on failure
  * \return a realtime boost::thread
  */
pthread create_thread(pthread_t *thread, int sched, int prio, void *(*start_routine) (void *), void *arg);

/** \brief set the realtime priority on a thread
  * \return true on success, false on failure
  */
bool set_realtime(pthread *thread);

}
}

#endif // RT_HPP
