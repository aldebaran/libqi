#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef AL_COMMON_MASTER_NODE_HPP_
#define AL_COMMON_MASTER_NODE_HPP_

#include <string>
#include <memory>

namespace AL {
  namespace Common {

    class MasterNodeImp;

    /// <summary> Master node. </summary>
    class MasterNode {
    public:

      /// <summary> Constructor. </summary>
      /// <param name="masterAddress">
      /// The master address.
      /// e.g. 127.0.0.1:5555
      /// </param>
      explicit MasterNode(const std::string& masterAddress);

      /// <summary> Finaliser. </summary>
      virtual ~MasterNode();

    private:
      std::auto_ptr<MasterNodeImp> fImp;
    };
  }
}

#endif  // AL_COMMON_MASTER_NODE_HPP_

