#ifndef SIG_GENERATOR_H
#define SIG_GENERATOR_H

#include <iostream>
#include <fstream>
#include <qi/application.hpp>
#include <boost/shared_ptr.hpp>

class SigGenerator
{
public:
  SigGenerator(int min_deep = 1, int max_deep = 5, int max_tuple_size = 8);
  std::string signature();

private:
  enum SigType {
    SigType_POD,
    SigType_COMPLEX,
    SigType_BOTH
  };

  //generate a random type (incomplete)
  char        genType(SigType sigtype = SigType_BOTH);
  std::string genCompleteType(SigType sigtype = SigType_BOTH);


private:
  int               min_deep;
  int               max_deep;
  int               max_tuple_size;
  int               current_deep;
};

#endif // SIG_GENERATOR_H
