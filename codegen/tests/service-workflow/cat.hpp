#include <qitype/anyobject.hpp>

namespace animals
{
// Helper struct
struct Mosquito
{
  double yaw,theta,distance;
};
class Cat
{
  public:
    virtual ~Cat() {}
    virtual void meow(int volume) = 0;
    virtual bool setTarget(const Mosquito& m) = 0;

    qi::Property<float> hunger;
    qi::Property<float> boredom;
    qi::Property<float> cuteness;
    qi::Signal<Mosquito> onTargetDetected;
};

}
