#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef _QI_SERIALIZATION_SERIALIZABLE_HPP_
#define _QI_SERIALIZATION_SERIALIZABLE_HPP_

namespace qi {
  namespace serialization {
    class Serializer;

    /// <summary>
    /// The Serializable Interface can be used to serialize or deserialize a struct.
    /// It forces you to implement a serialize method that receives
    /// a reference to a Serializer. You should make the serializer visit each member
    /// that you wish to be a part of your serialized structure.
    /// e.g.
    /// struct Point2D : Serializable {
    ///   int x;
    ///   int y;
    ///   void serialize(Serializer& s) {
    ///     s.visit(x, y);
    ///   }
    /// };
    /// </summary>
    class Serializable {
    public:

      /// <summary>
      /// Serializes to a message or desirializes from a message using the reference
      /// to the serializer which was constructed with a reference to a message.
      /// </summary>
      /// <param name="serializer"> [in,out] The serializer. </param>
      virtual void serialize(Serializer& serializer) = 0;
    };

  }
}

#endif  // _QI_SERIALIZATION_SERIALIZABLE_HPP_
