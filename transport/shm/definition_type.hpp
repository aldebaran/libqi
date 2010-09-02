/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef LIBIPPC_DEFINITIONTYPE_HPP_
#define LIBIPPC_DEFINITIONTYPE_HPP_

namespace AL {
  namespace Transport {

    enum DefinitionType {
      TypeCall = 1      << 0,
      TypeResult = 1    << 1,
      TypeException = 1 << 2
};

}
}

#endif /* !LIBIPPC_DEFINITIONTYPE_HPP_ */
