/*
**
** Author(s):
**  - Laurent LEC <llec@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <stdlib.h>
#include <string.h>
#include <qimessaging/qi.h>
#include <qimessaging/session.h>
#include <qimessaging/session.hpp>


qi_session_t *qi_session_create()
{
    qi::Session *session = new qi::Session();

    return reinterpret_cast<qi_session_t*>(session);
}

void qi_session_connect(qi_session_t *session, const char *address)
{
    qi::Session *s = reinterpret_cast<qi::Session*>(session);

    s->connect(address);
}

void qi_session_wait_for_connected(qi_session_t *session, int msecs)
{
    qi::Session *s = reinterpret_cast<qi::Session*>(session);

    s->waitForConnected(msecs);
}

/*
qi_object_t *qi_session_get_service(qi_session_t *session, const char *name)
{
    qi::Session *s = reinterpret_cast<qi::Session*>(session);
    qi::TransportSocket *socket = s->serviceSocket(name);

    return reinterpret_cast<qi_object_t*>(s->service(name));
}
*/
void qi_session_destroy(qi_session_t *session)
{
    qi::Session *s = reinterpret_cast<qi::Session*>(session);

    delete s;
}

void qi_session_disconnect(qi_session_t *session)
{
    qi::Session *s = reinterpret_cast<qi::Session*>(session);

    s->disconnect();
}

void qi_session_wait_for_disconnected(qi_session_t *session, int msecs)
{
    qi::Session *s = reinterpret_cast<qi::Session*>(session);

    s->waitForDisconnected(msecs);
}

const char** qi_session_get_services(qi_session_t *session)
{
    qi::Session *s = reinterpret_cast<qi::Session*>(session);
    std::vector<std::string> services = s->services();
    size_t length = services.size();
    const char **result = static_cast<const char**>(malloc((length + 1) * sizeof(char *)));
    unsigned int i = 0;

    for (i = 0; i < length; i++)
    {
        result[i] = strdup(services[i].c_str());
    }

    result[i] = NULL;

    return result;
}

void qi_session_free_services_list(const char **list)
{
    while (*list != 0)
    {
        free((void*) (*list));
    }

    free((void*) (*list));
}
