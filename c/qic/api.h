#pragma once
/*
 *  Copyright (C) 2011 Aldebaran Robotics
 */

#ifndef _QIMESSAGING_API_H_
#define _QIMESSAGING_API_H_

#include <qi/macro.hpp>

#define QIC_API QI_LIB_API(qimessaging)

typedef struct qi_application_t_s      {} qi_application_t;
typedef struct qi_future_t_s           {} qi_future_t;
typedef struct qi_promise_t_s          {} qi_promise_t;
typedef struct qi_object_t_s           {} qi_object_t;
typedef struct qi_object_builder_t_s   {} qi_object_builder_t;
typedef struct qi_servicedirectory_t_s {} qi_servicedirectory_t;
typedef struct qi_session_t_s          {} qi_session_t;
typedef struct qi_signal_t_s           {} qi_signal_t;
typedef struct qi_value_t_s            {} qi_value_t;

#endif  // _QIMESSAGING_API_H_

