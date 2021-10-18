#include "src/messaging/servicedirectory.hpp"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <ka/utility.hpp>
#include <ka/empty.hpp>

struct ServiceInitEndpointsAndRelativeEndpointExpectations
{
  std::vector<qi::Uri> serv1Endpoints;
  std::vector<qi::Uri> serv2Endpoints;
  bool serv1RelEpExpected;
  bool serv2RelEpExpected;
};

namespace
{

testing::Matcher<qi::Uri> IsRelativeEndpoint(std::string serv)
{
  using namespace testing;
  return AllOf(Property(&qi::Uri::scheme, qi::uriQiScheme()),
               Property(&qi::Uri::authority, Truly(ka::empty)),
               Property(&qi::Uri::path, serv));
}

struct ServiceDirectoryServiceRelativeEndpoint
  : testing::TestWithParam<
      // UseFeature x UseServices x ServiceInitEndpointsAndRelativeEndpointExpectations
      std::tuple<bool,
                 bool,
                 ServiceInitEndpointsAndRelativeEndpointExpectations>>
{
  ServiceDirectoryServiceRelativeEndpoint()
    : sbo(makeServiceBoundObjectPtr(qi::Message::Service_ServiceDirectory,
                                    qi::AnyObject{},
                                    qi::MetaCallType_Direct))
  {
    sd._setServiceBoundObject(sbo);

    const auto servInitEpAndRelExpect = std::get<2>(GetParam());
    const auto serv1InitEndpoints = servInitEpAndRelExpect.serv1Endpoints;
    const auto serv2InitEndpoints = servInitEpAndRelExpect.serv2Endpoints;
    addService(service1, serv1InitEndpoints);
    addService(service2, serv2InitEndpoints);
  }

  void addService(std::string name, std::vector<qi::Uri> endpoints)
  {
    qi::ServiceInfo info;
    info.setName(ka::mv(name));
    info.setMachineId(qi::os::getMachineId());
    info.setProcessId(static_cast<unsigned int>(qi::os::getpid()));
    info.setEndpoints(ka::mv(endpoints));
    sd.serviceReady(sd.registerService(info));
  }

  qi::RelativeEndpointsUriEnabled feature() const
  {
    return std::get<0>(GetParam()) ? qi::RelativeEndpointsUriEnabled::Yes :
                                     qi::RelativeEndpointsUriEnabled::No;
  }

  std::vector<qi::Uri> endpoints(const std::string& serv)
  {
    const auto useServices = std::get<1>(GetParam());
    if (useServices)
    {
      const auto services = sd.services(feature());
      const auto it = std::find_if(services.begin(), services.end(), [&](const qi::ServiceInfo& info) {
        return info.name() == serv;
      });
      if (it == services.end())
      {
        ADD_FAILURE() << "Could not find service " << serv << " in services list.";
        return {};
      }
      return it->uriEndpoints();
    }
    return sd.service(serv, feature()).uriEndpoints();
  }

  bool serv1ExpectedRelEp() const { return std::get<2>(GetParam()).serv1RelEpExpected; }
  bool serv2ExpectedRelEp() const { return std::get<2>(GetParam()).serv2RelEpExpected; }

  qi::BoundObjectPtr sbo;
  qi::ServiceDirectory sd;
  const std::string service1 = "cookies";
  const std::string service2 = "muffins";
};

// Services init endpoints and expectations of relative endpoints
ServiceInitEndpointsAndRelativeEndpointExpectations servInitEpAndRelEpExpect[] = {
  // Endpoints equality, both service info contain a relative endpoint to the other one as
  // they share the same endpoints.
  {
    { *qi::uri("tcp://1.2.3.4:1"), *qi::uri("tcp://localhost:2") },
    { *qi::uri("tcp://1.2.3.4:1"), *qi::uri("tcp://localhost:2") },
    true, true,
  },

  // First service contains more endpoints than the second one, but all of the seconds are
  // included in the first, therefore the first service can be contacted through a relative
  // endpoint to the second.
  {
    { *qi::uri("tcp://10.20.30.40:2"), *qi::uri("tcp://localhost:1"),
      *qi::uri("tcp://10.11.12.13:3") },
    { *qi::uri("tcp://10.20.30.40:2"), *qi::uri("tcp://localhost:1") },
    true, false
  },

  // Services share a common endpoint, but no service includes all the endpoints of another,
  // therefore no relative endpoint is added.
  {
    { *qi::uri("tcp://localhost:1"), *qi::uri("tcp://20.21.22.23:2") },
    { *qi::uri("tcp://localhost:1"), *qi::uri("tcp://30.31.32.33:3") },
    false, false
  },

  // There is no intersection between the endpoints of the two services, no relative endpoint is
  // added.
  {
    { *qi::uri("tcp://20.21.22.23:10") },
    { *qi::uri("tcp://10.20.30.40:20") },
    false, false
  }
};

INSTANTIATE_TEST_SUITE_P(
  FeatureSwitch,
  ServiceDirectoryServiceRelativeEndpoint,
  testing::Combine(
    testing::Bool(), // Use the feature.
    testing::Bool(), // Use the `services` function.
    testing::ValuesIn(servInitEpAndRelEpExpect)
  )
);

} // anonymous namespace

TEST_P(ServiceDirectoryServiceRelativeEndpoint, RelativeEndpointsArePresentWhenExpected)
{
  using namespace testing;
  const auto serv1Ep = endpoints(service1);
  const auto serv2Ep = endpoints(service2);

  // When the feature is disabled, there must never be a relative endpoint.
  const auto hasFeature = feature() == qi::RelativeEndpointsUriEnabled::Yes;

  // Create matchers depending on the expected outcome (whether or not we expect relative endpoints,
  // and whether or not the feature is enabled). Either the service endpoints MUST contain at least
  // one relative endpoint to the other service, or it must contain none.
  const auto matchesExpectation = [](bool mustContainRelEp,
                                     const std::string& serv) -> Matcher<std::vector<qi::Uri>> {
    if (mustContainRelEp)
      return Contains(IsRelativeEndpoint(serv));
    return Not(Contains(IsRelativeEndpoint(serv)));
  };
  const auto serv1MatchesExpectation =
    matchesExpectation(hasFeature && serv1ExpectedRelEp(), service2);
  const auto serv2MatchesExpectation =
    matchesExpectation(hasFeature && serv2ExpectedRelEp(), service1);

  EXPECT_THAT(serv1Ep, serv1MatchesExpectation);
  EXPECT_THAT(serv2Ep, serv2MatchesExpectation);
}

TEST_P(ServiceDirectoryServiceRelativeEndpoint, EndpointsAreSorted)
{
  using namespace testing;
  const auto serv1Ep = endpoints(service1);
  const auto serv2Ep = endpoints(service2);
  EXPECT_THAT(serv1Ep, WhenSortedBy(&qi::isPreferredEndpoint, serv1Ep));
  EXPECT_THAT(serv2Ep, WhenSortedBy(&qi::isPreferredEndpoint, serv2Ep));
}
