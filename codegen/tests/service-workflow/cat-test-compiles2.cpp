#include <generated/cat.hpp>

class Lynx: public animals::Cat
{
public:
  void meow(int64_t volume) {}
  bool setTarget(const animals::Mosquito& m) { return true;}
  qi::Object<animals::CatAction> selectTask(qi::int64_t seed) { return qi::Object<animals::CatAction>();}
  bool canPerform(qi::Object<animals::CatAction> task) { return true;}
};

