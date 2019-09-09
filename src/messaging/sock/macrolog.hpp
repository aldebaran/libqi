#include <qi/log.hpp>
#include "option.hpp"

#define QI_LOG_ERROR_SOCKET(SOCKET_PTR) qiLogError(sock::logCategory()) << (SOCKET_PTR) << ": "
#define QI_LOG_WARNING_SOCKET(SOCKET_PTR) qiLogWarning(sock::logCategory()) << (SOCKET_PTR) << ": "
#define QI_LOG_VERBOSE_SOCKET(SOCKET_PTR) qiLogVerbose(sock::logCategory()) << (SOCKET_PTR) << ": "
#define QI_LOG_DEBUG_SOCKET(SOCKET_PTR) qiLogDebug(sock::logCategory()) << (SOCKET_PTR) << ": "
