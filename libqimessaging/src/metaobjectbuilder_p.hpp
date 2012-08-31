/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef   	METAOBJECTBUILDER_P_HPP_
# define   	METAOBJECTBUILDER_P_HPP_

namespace qi {

  class MetaObject;
  class MetaObjectBuilderPrivate {
  public:
    MetaObjectBuilderPrivate(MetaObject *meta)
      : _metaObject(meta)
    {}

    //not owned by us
    MetaObject *_metaObject;
  };
}

#endif	    /* !METAOBJECTBUILDER_P_PP_ */
