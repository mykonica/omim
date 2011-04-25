#pragma once

#include "../std/target_os.hpp"

#if defined(OMIM_OS_BADA)
  #include <FBaseRtThreadMonitor.h>
#else
  #include "mutex.hpp"
  #if !defined(OMIM_OS_WINDOWS_NATIVE)
    #include <pthread.h>
  #endif
#endif

namespace threads
{
  /// Implements mutexed condition semantics
  class Condition
  {
#if defined(OMIM_OS_BADA)
    Osp::Base::Runtime::Monitor m_Monitor;
#else
    Mutex m_Mutex;
  #if defined(OMIM_OS_WINDOWS_NATIVE)
    CONDITION_VARIABLE m_Condition;
  #else
    pthread_cond_t m_Condition;
  #endif
#endif

  public:
    Condition()
    {
#if defined(OMIM_OS_BADA)
      m_Monitor.Construct();
#elif defined(OMIM_OS_WINDOWS_NATIVE)
      ::InitializeConditionVariable(&m_Condition);
#else
      ::pthread_cond_init(&m_Condition, 0);
#endif
    }

    ~Condition()
    {
#if !defined(OMIM_OS_WINDOWS_NATIVE) && !defined(OMIM_OS_BADA)
      // only for pthreads
      ::pthread_cond_destroy(&m_Condition);
#endif
    }

    void Signal()
    {
#if defined(OMIM_OS_BADA)
      m_Monitor.Notify();
#elif defined(OMIM_OS_WINDOWS_NATIVE)
      ::WakeConditionVariable(&m_Condition);
#else
      ::pthread_cond_signal(&m_Condition);
#endif
    }

    void Wait()
    {
#if defined(OMIM_OS_BADA)
      m_Monitor.Wait();
#elif defined(OMIM_OS_WINDOWS_NATIVE)
      ::SleepConditionVariableCS(&m_Condition, &m_Mutex.m_Mutex, INFINITE);
#else
      ::pthread_cond_wait(&m_Condition, &m_Mutex.m_Mutex);
#endif
    }

    void Lock()
    {
#if defined(OMIM_OS_BADA)
      m_Monitor.Enter();
#else
      m_Mutex.Lock();
#endif
    }

    void Unlock()
    {
#if defined(OMIM_OS_BADA)
      m_Monitor.Exit();
#else
      m_Mutex.Unlock();
#endif
    }
  };

  /// ScopeGuard wrapper around mutex
  class ConditionGuard
  {
  public:
    ConditionGuard(Condition & condition): m_Condition(condition) { m_Condition.Lock(); }
    ~ConditionGuard() { m_Condition.Unlock(); }
    void Wait() { m_Condition.Wait(); }
    void Signal() { m_Condition.Signal(); }
  private:
    Condition & m_Condition;
  };
}
