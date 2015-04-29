
//Eventloop->post  (with delay = 0)
TRACEPOINT_EVENT(qi_qi, eventloop_post,
        TP_ARGS(unsigned int, taskId,
                const char*, typeName),
        TP_FIELDS(ctf_integer(int, taskId, taskId)
                  ctf_string(typeName, typeName)
                  )
)

//Eventloop->async && post with delay
TRACEPOINT_EVENT(qi_qi, eventloop_delay,
        TP_ARGS(unsigned int, taskId,
                const char*, typeName,
                unsigned int, usDelay),
        TP_FIELDS(ctf_integer(int, taskId, taskId)
                  ctf_string(typeName, typeName)
                  ctf_integer(int, usDelay, usDelay))
)

//task really start
TRACEPOINT_EVENT(qi_qi, eventloop_task_start,
        TP_ARGS(unsigned int, taskId),
        TP_FIELDS(ctf_integer(int, taskId, taskId))
)

//task stop
TRACEPOINT_EVENT(qi_qi, eventloop_task_stop,
        TP_ARGS(unsigned int, taskId),
        TP_FIELDS(ctf_integer(int, taskId, taskId))
)

//task has been canceled before running
TRACEPOINT_EVENT(qi_qi, eventloop_task_cancel,
        TP_ARGS(unsigned int, taskId),
        TP_FIELDS(ctf_integer(int, taskId, taskId))
)

//task has been set on error
TRACEPOINT_EVENT(qi_qi, eventloop_task_error,
        TP_ARGS(unsigned int, taskId),
        TP_FIELDS(ctf_integer(int, taskId, taskId))
)
