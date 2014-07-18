#ifndef   	CAT_HPP_
# define   	CAT_HPP_

#include <qi/anyobject.hpp>
#include <qi/property.hpp>

class Cat
{
public:
    Cat() { meowVolume.set(0xCA72); }
    Cat(unsigned int val) { meowVolume.set(val); }

    int meow() const { return meowVolume.get(); }

    qi::Property<unsigned int> meowVolume;
};

#endif	    /* !CAT_PP_ */
