/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Cedric GESTES
*/

#ifndef   	AL_MESSAGING_CLIENT_HXX_
# define   	AL_MESSAGING_CLIENT_HXX_

namespace AL {
  namespace Messaging {


    AL::ALPtr<ResultDefinition> Client::send(CallDefinition &def)
    {
      AL::ALPtr<ResultDefinition> ret(new ResultDefinition());

      //TODO: avoid a maximum of copy. (maybe use a custom message type here)

      //boost::interprocess::bufferstream bstream((char *)msg.data(), msg.size());
      std::stringstream           strstream;
      std::string                 result;

      //TODO: use the abstrated serialization framework here
      OArchive                    oarchive(strstream);
      oarchive << def;

      _transport->send(strstream.str(), result);


      //TODO: use the abstracted serialization framework here
      boost::interprocess::bufferstream bstream((char *)result.data(), result.size());
      IArchive                          archive(bstream);
      archive >> (*ret);
      return ret;
    }


    template<typename R, typename T1>
    R Client::call(const P1 &p1)
    {
    }

    template<typename R, typename T1, typename T2>
    R Client::call(const P1 &p1, const P2 &p2)
    {
    }

  }
}


#endif	    /* !AL_MESSAGING_CLIENT_HXX_ */
