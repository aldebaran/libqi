#ifndef CAT_HPP
#define CAT_HPP
#include <qitype/anyobject.hpp>
#include <qitype/property.hpp>

namespace animals
{
// Helper struct
struct Mosquito
{
  double yaw,theta,distance;
};

/// Possible action a cat can perform
class CatAction
{
public:
  /// Run the action
  virtual void run() = 0;
  virtual std::vector<float> expectedResult() = 0;
};

/** Interface to a cat */
class Cat
{
  public:
    virtual ~Cat() {}
    virtual void meow(int volume) = 0;
    virtual bool setTarget(const Mosquito& m) = 0;
    virtual qi::Object<CatAction> selectTask() = 0;
    virtual bool canPerform(qi::Object<CatAction> task) = 0;

    qi::Property<float> hunger;
    qi::Property<float> boredom;
    qi::Property<float> cuteness;
    qi::Signal<Mosquito> onTargetDetected;
};

}
#endif
