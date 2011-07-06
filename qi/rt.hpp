#ifndef RT_HPP
#define RT_HPP

#include <ctime>

namespace qi {
namespace rt {



/** \brief create an rt thread
  * throw qi::os_error on failure
  *
  * policy values (from pthread documentation):
  * SCHED_FIFO
  *   First in-first out (FIFO) scheduling policy. (need realtime capabilities)
  * SCHED_RR
  *   Round robin scheduling policy. (need realtime capabilities)
  * SCHED_SPORADIC
  *   Sporadic server scheduling policy.
  * SCHED_OTHER
  *   Another scheduling policy.
  * \param thread return the pthread id
  * \param policy scheduler policy
  * \param rtprio the realtime priority (unused if not applicable)
  * \param start_routine the function to start in the thread
  * \param arg a pointer to an opaque data member passed in parameter to the start_routine
  * \return a realtime boost::thread
  */
int realtime_thread_create(pthread_t *thread, int policy, int rtprio, void *(*start_routine) (void *), void *arg);

/** \brief runnable interface
 */
class runnable {
  /** \brief the run() virtual method that must be implemented by derived class
   */
  virtual void run() = 0;
};


/** \brief create a pthread realtime thread.
 * \param prunnable a pointer to an instance of a class that implement the runnable interface.
 */
int realtime_thread_create(pthread_t *thread, int policy, int rtprio, runnable *prunnable);

/** \brief set the realtime priority on a thread
  * \return true on success, false on failure
  */
bool set_realtime(pthread_t *thread);

}
}

#endif // RT_HPP
