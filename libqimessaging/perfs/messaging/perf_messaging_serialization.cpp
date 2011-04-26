
#include <gtest/gtest.h>
#include <string>
#include <qi/messaging.hpp>
#include <qi/perf/sleep.hpp>
#include <qi/perf/dataperftimer.hpp>

#include <qimessaging/serialization.hpp>
#include <qimessaging/serialization/serializable.hpp>
#include <qimessaging/serialization/serializer.hpp>
#include <qimessaging/serialization/boost/boost_serializers.hpp>
#include <math.h>

using namespace qi;
using namespace qi::serialization;
const unsigned int kNumPowers = 12;
const unsigned int kNumLoops = 1000000;

TEST(Serialization, qiVectorFloat)
{
  qi::perf::DataPerfTimer dt;
  for (unsigned int k = 0; k < kNumPowers; k++) {
    unsigned int kNumFloats = (int)powf((float)2, k);
    std::vector<float> f;
    f.assign( kNumFloats, 1.1111098340f);
    dt.start(kNumLoops, kNumFloats * sizeof(float));
    for (unsigned int i=0; i<kNumLoops; i++) {
      qi::serialization::Message m;
      qi::serialization::serialize<std::vector<float> >::write(m, f);
      //std::cout << m.str().size() << "---" << m.str() << std::endl;
    }
    dt.stop();
  }
}



struct QiFloatStruct : qi::serialization::Serializable {
  std::vector<float> f;

  void accept(qi::serialization::Serializer& s) {
    s.visit(f);
  }
};

TEST(Serialization, qiFloatStruct)
{
  qi::perf::DataPerfTimer dt;
  for (unsigned int k = 0; k < kNumPowers; k++) {
    unsigned int kNumFloats = (int)powf((float)2, k);
    std::vector<float> f;
    f.assign( kNumFloats, 1.1111098340f);

    QiFloatStruct qfs;
    qfs.f = f;
    dt.start(kNumLoops, kNumFloats * sizeof(float));
    for (unsigned int i=0; i<kNumLoops; i++) {
      Message m;
      Serializer s(ACTION_SERIALIZE, m);
      qfs.accept(s);
      //std::cout << m.str().size() << "---" << m.str() << std::endl;
    }
    dt.stop();
  }
}

TEST(Serialization, boostVectorFloat)
{
  qi::perf::DataPerfTimer dt;
  for (unsigned int k = 0; k < kNumPowers; k++) {
    unsigned int kNumFloats = (int)powf((float)2, k);
    std::vector<float> f;
    f.assign( kNumFloats, 1.1111098340f);

    dt.start(kNumLoops, kNumFloats * sizeof(float));
    for (unsigned int i=0; i<kNumLoops; i++) {
      std::string s = qi::serialization::BoostMessage::serialize(f);
      //std::cout << s.size() << "---" << s << std::endl;
    }
    dt.stop();
  }
}

struct BoostFloatStruct {
  std::vector<float> f;

  template <typename Archive>
  void serialize(Archive& a, const unsigned int version) {
    a & f;
  }
};

TEST(Serialization, boostFloatStruct)
{
  qi::perf::DataPerfTimer dt;
  for (unsigned int k = 0; k < kNumPowers; k++) {
    unsigned int kNumFloats = (int)powf((float)2, k);
    std::vector<float> f;
    f.assign( kNumFloats, 1.1111098340f);

    BoostFloatStruct bfs;
    bfs.f = f;

    dt.start(kNumLoops, kNumFloats * sizeof(float));
    for (unsigned int i=0; i<kNumLoops; i++) {
      std::string s = qi::serialization::BoostMessage::serialize(bfs);
      //std::cout << s.size() << "---" << s << std::endl;
    }
    dt.stop();
  }
}

#include "vfloat.pb.h"

TEST(Serialization, pbFloatStruct)
{
  qi::perf::DataPerfTimer dt;
  for (unsigned int k = 0; k < kNumPowers; k++) {
    unsigned int kNumFloats = (int)powf((float)2, k);
    VFloat vf;
    for (unsigned int i=0; i < kNumFloats; i++) {
      vf.add_f(1.1111098340f);
    }


    dt.start(kNumLoops, kNumFloats * sizeof(float));
    for (unsigned int i=0; i<kNumLoops; i++) {
      //vf.set_f(20, 1.78789f * i);
      std::string s = vf.SerializeAsString();
      //std::cout << s.size() << "---" << std::endl << s << std::endl;
    }
    dt.stop();
  }
}
