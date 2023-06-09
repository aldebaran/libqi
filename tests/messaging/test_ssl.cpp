/*
**  Copyright (C) 2022 SoftBank Robotics Europe
**  See COPYING for the license
*/

#include <qi/messaging/ssl/ssl.hpp>
#include <ka/conceptpredicate.hpp>
#include <boost/filesystem.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <fstream>

namespace ssl = qi::ssl;

const char* const passphrase = "framework";

// See the `generate_certs.py` for how these are generated.

const std::string keyData =
  "-----BEGIN PRIVATE KEY-----\n"
  "MIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQDQu/YwwBP+OnoE\n"
  "LchrYVsGwSzlhT8lHXoNzdHGz2HQda4DWbA/nO3ikTHrycpwxErchdU9Guhg9DWa\n"
  "F0+mBi574otQuTmNEgXRSnXAulIa8uSR/OlihP8H7mhzxJCsLtmDjR8TozMiPr+z\n"
  "i7sx4eUemmdhVLOEZIVTY374BxDl9UKf4z17IjvcV8NKvbspmkXKCoMzCwK9zCBz\n"
  "LHDVOBxbuVCp0pRGULovGBbzJ7GtiotiA+PGkWHstA1NE9isxF7S7KO9oIcAj4Ni\n"
  "WzkzlrFV5RSKT9UHWPeOPRDr9l9E0vQx/iONa624M8w9PzkmZrb5bfy5EC5oetF5\n"
  "xhsabcWhAgMBAAECggEAewoA0bW3U0lW+TWfiSRnCrE97MFBenhIpPBosl82EjH7\n"
  "8/+fM7AmTUI3Afz9tsEOO7UsAAvnZJcvLXiGO6AzMFVWt6unL++qvDa++G8bzMTW\n"
  "VPOXArrQtfUjXvJEMSqgjrnV0raFgq8oQwafvoSgkQyE2cua0fSK3p+sjTllDZcX\n"
  "BPmPajuXFLq4EmGG0NaRJmZXgPRQiELCLVU7nkvCcYtp5/vQOlW+t+f/zJLzcrBI\n"
  "4clOTjF7HCNncioMKVQyFFsG98SUfsc+OPoPZgsJgVin3YuxmhO2UH23V4As9E7a\n"
  "7IANL4snnAe/8TycrUWs+Kk99msIzjUiIdA+LYsdUQKBgQDwhEgv4pVrKgwNDrE1\n"
  "T8vQQKqj4EdkSHv8xzQBBIi+HIhA5ajrg62uFEsUqXuqCI3Y7yIMxSTIsmkKeTR6\n"
  "/lAfAT93yi80LdAb2FKW5ENAtFl7bh6kM9bDOeetjrWu3x02QBtLP+NT/vtyjdqF\n"
  "Uzw3jgrnSYDTwnbIT9DGWOlW9QKBgQDeK+ddYkvgif4sRvIvx4wPA3oJHdf8jha3\n"
  "F0uhRiz8z5qXJjHjaBGONFKPkvPshEkdEtNQNNsyp2nEz2w0Bdv4I9ChKsyh6xpY\n"
  "E9D6yk2hmb6XBXcTdS49f+QtTScbSM/MCagYAk2LEMH+7mCgpc2fGZj+yaGpl4zD\n"
  "rPRX/3MQfQKBgCc3ioW0TmTA4GIoqSBmOHdmc+xwn3NsljrrSEF6Ocm14UmjqN1u\n"
  "BR3DVVKzJ+TbrDVZGJY4dvz6ikIY7UO4v9uuWmjv4K6DYdGIfj74TTb5sTV9CSYB\n"
  "Bd2jTXFxOZKf8qVr4odsjWaexuUw2BaxMbplQMXhqE3fAXs7+HC/Ap0ZAoGBAJcd\n"
  "48xoDNZY5jL1usv2/fREWCCVVRErEJZO2RmEYxs+lpcS2sIkSjSgsDuKKDILNLP6\n"
  "1xLqMOJ+bvn4YE/uYFWi/shxFSlZgdzA9ddv/Wfa7IFKrVjlzqL0N578qIntd7IA\n"
  "K37RKi9aIaomOEFtJbR/M8qyGS0CsTNzk+u7DGf9AoGAX/qnTEzKcNVMbRrNRY6H\n"
  "JO1/TC5zQNepZJ9qEP3GTexOoWgg+Ps0gE2bqqo/AuPUehojg4/QtbG2OsLmhwjA\n"
  "193XjoVj/A/tRmDJydAAxa2yIKWCXcZhiOoqge95kBqXfJDBw+cgLqu3W2TajVtD\n"
  "QTfvT4OvKNIc2EfKBLCrRLA=\n"
  "-----END PRIVATE KEY-----\n";

const std::string encryptedKeyData =
  "-----BEGIN ENCRYPTED PRIVATE KEY-----\n"
  "MIIFHDBOBgkqhkiG9w0BBQ0wQTApBgkqhkiG9w0BBQwwHAQIVZeE6W5GbUACAggA\n"
  "MAwGCCqGSIb3DQIJBQAwFAYIKoZIhvcNAwcECIwiweyAKYM/BIIEyE3Tfcd2rvXV\n"
  "DyHAOqZ6Y5Rw+KVaFZ1jcLoCelQSSVdFftx8pT35iMOWiwYYcEqETuI0+8NY9ZtS\n"
  "dAEG8OjSK1xuAlWqH1vRtLpfVrHrQCYqQxThP8V9HEjuqrjcT1Cfn/IRAzTNoJU9\n"
  "Zxk8AKmwmyr3Ey6tjD22pxQZTmQWn47NCfFxMZEdBD+QPmYfZiOYNJs9p6s6TneR\n"
  "c4kLmwbrZPlCUJezVP0psDQEws9O493qVdcnNGrKn0uWov1c2EeAZKez3jeElAtK\n"
  "JHKUcC2i/A7xADbrgj3gJ3Puq6xYDCJATNLF0zXFtRHhLjuhXLoO5Rw+be+hq8DE\n"
  "qoRqE1XCcqTijDGtjG0AYjBhw9n3MVanhaTaZf+gdMuQ6QuE4G55nh6W2OdJEfbY\n"
  "ggZQ1tSDWq5/jGossLPVQrydL5aiNs11N4KjAMU/d/jrkxrQcAo2g5KJuU6fnute\n"
  "ypdMQmtqkMnILOUDs8XZpkDGhyKsVCSv/5qQM84eCc6zLfFgSSer1mq9i4bEmlRQ\n"
  "wSnqBCdI612cPFJddcFRWMdpUcAxbu9TstrAvKq84zq+byYsKmUTueDLu1TQoL2E\n"
  "PTPHk92YTZUPfDhHqtNAriGKbG4tn+unsI8K2ORWH+Kz9TC5d3OBj/c5mPUoDGeR\n"
  "41PgBYvVXZyG2NxH45CSQT2Xo065zOSdKEXyxVP6vpLt3GUZoLjP2Mg/zze/BFUN\n"
  "Fl0x28pOa62yDi4GWVsp4h3BQQRuIwa8e/gthe0sWjsmdb+j2ajBB8KjMgkr+Wzf\n"
  "zb8ewJjvwgrmGlx5emAVmY9wY6ulPQtIbayIRxj1toaVKOSRwZCV8PiW7SWgLizo\n"
  "ZpdD3qJjH1CF+h+6P6raMW/NN/x05DMQ7FwZ2o41S6P/DrBqtUCTJ27YT0vBvUpd\n"
  "eNsIRXfH12C/erUyK2ZYCqnHhF/M616JhQupPKZZWP5svnu1xnKoaF6JKVQ4cdri\n"
  "P9O74p9NCT6d/Vwst8LDxfP/96a2mnGsVIvqHuhLE4b20k/1cQjlSfVHoUvAGYlK\n"
  "o7Xy78Cs3GeSgzJIHKpSd6903e9dcFLReSoffnM0+fNRW8lwNyxFCE4V8kMugYcR\n"
  "DyWdDwNE/zJE61j+lnJRj3EED16O/e8llKSE7YOddy3Y7Y1mOjzYrY+QN94SmWwT\n"
  "PhtboH+lGVxLclZPRLHSDrwT5q8MIJtSBiCgcCBLk0KI3ZAeAfrM/QJTb8HvGlvB\n"
  "5/kyMU5gsX54NgCvjEB/FN4OfDBfhvR0gy98LpDOkH37BNC36vBor3Y/AlUUvgkk\n"
  "45neE2aPS9bK1Nx/qk9BZVOQQ8V8ArWE8hXejVCUCYq3MMJG9jq1lZK1b/IDRpUY\n"
  "hMJz40ujtNkDHaQlIJFrfPJYMPQzSnf9QcMi8QyUbcV5vGUvaGo7p7HgyTn51qd5\n"
  "/hMbRlBRCDDSPl8XPzkWT/uuc7ZVRIJJkSAekcgCAHLzwOMPkxcCsr5EFWUZuVt7\n"
  "4N0Eybw/Bi+BvZ04sKvYy2TMjq69RPg8XwYffwlw39wmyf6KwSjmS6bgnbV0RbP8\n"
  "nPB930KhTvXCV8Xe7+rQS2I9JSkhnsEhp+qQqtuxpsCQ+YpM+GYYdoM+VH8SCvyk\n"
  "dUbEtMvR8cMzuqdVY5mWJA==\n"
  "-----END ENCRYPTED PRIVATE KEY-----\n";

const std::string invalidKeyData =
  "-----BEGIN PRIVATE KEY-----\n"
  "not a valid private key\n"
  "-----END PRIVATE KEY-----\n";

const std::string cert1Data =
  "-----BEGIN CERTIFICATE-----\n"
  "MIIEPjCCAyagAwIBAgIUO6zAyDb3GABLO/U+zWnndjTy+w8wDQYJKoZIhvcNAQEL\n"
  "BQAwfTELMAkGA1UEBhMCRlIxDjAMBgNVBAgMBVBhcmlzMQ4wDAYDVQQHDAVQYXJp\n"
  "czESMBAGA1UECgwJQWxkZWJhcmFuMRcwFQYDVQQLDA5GcmFtZXdvcmsgVGVhbTEh\n"
  "MB8GA1UEAwwYdGVzdC5saWJxaS5sb2NhbCBSb290IENBMB4XDTIyMDMxNTE0MDg0\n"
  "NloXDTQ5MDczMTE0MDg0NlowfDELMAkGA1UEBhMCRlIxDjAMBgNVBAgMBVBhcmlz\n"
  "MQ4wDAYDVQQHDAVQYXJpczESMBAGA1UECgwJQWxkZWJhcmFuMRcwFQYDVQQLDA5G\n"
  "cmFtZXdvcmsgVGVhbTEgMB4GA1UEAwwXdGVzdC5saWJxaS5sb2NhbCBTZXJ2ZXIw\n"
  "ggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQC56mgaz9HlHC9HVJLpQRZJ\n"
  "EKIVRVt9bKHKAKMUTiTcF3t8r0ivXgJ1vOUc49R5j8MikSD38yFHkX9h1IOLptNX\n"
  "1jJYxIWgo80DICrUGOSDvcSU0nOyiE3J3MEdYWJrCP5usuTXVqpx+r075Y3Atz0N\n"
  "k1Kh5O49C66NWN6BR865i+cjo9daJynYdBFSw1Xf19V3mFeo3D9KyXU1HrGqrkLd\n"
  "VfTLSA6LaZT52tiEnlqK6qrMQtp3GEDnsHoPz3DPsEZE5ztCIChraJ6khI938FYH\n"
  "4CSxCtrJFbQ5Uv2Q6kTnmUifz7JLpAV2lVMYqMp1rUDYXIro0/JvJcMFav2wyj+/\n"
  "AgMBAAGjgbYwgbMwHwYDVR0jBBgwFoAUn5ttKD7d0kwKA4kS+jeO+g1dP5QwCQYD\n"
  "VR0TBAIwADAOBgNVHQ8BAf8EBAMCA/gwHQYDVR0OBBYEFMyZ/MaepF1HlA3bBdvF\n"
  "02xg4N7LMD4GA1UdEQQ3MDWHBH8AAAGHEAAAAAAAAAAAAAAAAAAAAAGCCWxvY2Fs\n"
  "aG9zdIIQdGVzdC5saWJxaS5sb2NhbDAWBgNVHSUBAf8EDDAKBggrBgEFBQcDATAN\n"
  "BgkqhkiG9w0BAQsFAAOCAQEAjnJ1g2YUp7ssmuTf1eh5G85n3cElJ1xfwxLEv4yo\n"
  "GiUD1cV1KJFdyLi3qYLEyh4oO//8S7pND7UnCEnzT3qu++6I4t8fRvyJlCjmKjay\n"
  "3Mmfz76jPbsnVnbf9LtQdyYRI3zkyEMw4sIRZYEo+oWCFOrHaRsflq4Ywfzj0ypK\n"
  "67qWYbUT7ZW6J+4yuCYX/AI6UkZfeCVgm/SiHB7Z7vd6s74pM9LQ1shJOzLS1hPX\n"
  "6e/U28QaNptlrza14XRZL2w1RWFcokrPc6T9mA26E3ivQhBvQuQN0VVOTa/sN5RY\n"
  "9mPdAb6hBJoOyKBI0mJL+ovQzMA46CR5yu09qSm/NoQ2kw==\n"
  "-----END CERTIFICATE-----\n";

const std::string cert2Data =
  "-----BEGIN CERTIFICATE-----\n"
  "MIID/DCCAuSgAwIBAgIUO6zAyDb3GABLO/U+zWnndjTy+xAwDQYJKoZIhvcNAQEL\n"
  "BQAwfTELMAkGA1UEBhMCRlIxDjAMBgNVBAgMBVBhcmlzMQ4wDAYDVQQHDAVQYXJp\n"
  "czESMBAGA1UECgwJQWxkZWJhcmFuMRcwFQYDVQQLDA5GcmFtZXdvcmsgVGVhbTEh\n"
  "MB8GA1UEAwwYdGVzdC5saWJxaS5sb2NhbCBSb290IENBMB4XDTIyMDMxNTE0MDg0\n"
  "NloXDTQ5MDczMTE0MDg0NlowfDELMAkGA1UEBhMCRlIxDjAMBgNVBAgMBVBhcmlz\n"
  "MQ4wDAYDVQQHDAVQYXJpczESMBAGA1UECgwJQWxkZWJhcmFuMRcwFQYDVQQLDA5G\n"
  "cmFtZXdvcmsgVGVhbTEgMB4GA1UEAwwXdGVzdC5saWJxaS5sb2NhbCBDbGllbnQw\n"
  "ggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDQu/YwwBP+OnoELchrYVsG\n"
  "wSzlhT8lHXoNzdHGz2HQda4DWbA/nO3ikTHrycpwxErchdU9Guhg9DWaF0+mBi57\n"
  "4otQuTmNEgXRSnXAulIa8uSR/OlihP8H7mhzxJCsLtmDjR8TozMiPr+zi7sx4eUe\n"
  "mmdhVLOEZIVTY374BxDl9UKf4z17IjvcV8NKvbspmkXKCoMzCwK9zCBzLHDVOBxb\n"
  "uVCp0pRGULovGBbzJ7GtiotiA+PGkWHstA1NE9isxF7S7KO9oIcAj4NiWzkzlrFV\n"
  "5RSKT9UHWPeOPRDr9l9E0vQx/iONa624M8w9PzkmZrb5bfy5EC5oetF5xhsabcWh\n"
  "AgMBAAGjdTBzMB8GA1UdIwQYMBaAFJ+bbSg+3dJMCgOJEvo3jvoNXT+UMAkGA1Ud\n"
  "EwQCMAAwDgYDVR0PAQH/BAQDAgP4MB0GA1UdDgQWBBS82wSy5CQkjArIj7wcimER\n"
  "n7bkFDAWBgNVHSUBAf8EDDAKBggrBgEFBQcDAjANBgkqhkiG9w0BAQsFAAOCAQEA\n"
  "qbsf+Q9IOMzhmo4ChQ/X+9ckIXT7srUC114Xv9esPzBFQyNUkaqyS0WqHUKwE/cO\n"
  "Nx+EppuJ4zlupGcgmunbucsyrvZy9gQO0+p596W4sr5D7Lm/Z8t/aFlyv867hZVB\n"
  "rMOcFQSpXdex+XgF46/ETKDk6LVCuJE2urvZg/vteW9qErMo+dG+oBPm6jC0buTC\n"
  "rOZFE48V848mnBWNFam9w2rk5fSfdU6nQ1FullYatIXA4YPy/ai02I1i7Xn/++n1\n"
  "4XSLKZkZIUFRflhaA9IM7ZHLNexhMbPaqPAN8NEp0yo/fn9PjD/9Yj65BSKE28BE\n"
  "cegyERtpdnhj9AuGveZqGA==\n"
  "-----END CERTIFICATE-----\n";

const std::string invalidCertData =
  "-----BEGIN CERTIFICATE-----\n"
  "not a valid certificate\n"
  "-----END CERTIFICATE-----\n";

/// Writes a new temporary file with the given content and returns the path of the file.
static boost::filesystem::path writeTmpFile(const std::string& content)
{
  const auto filename = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
  std::ofstream ofs(filename.string());
  EXPECT_TRUE(ofs);
  ofs << content;
  EXPECT_TRUE(ofs);
  return filename;
}

TEST(SslError, FromCurrentErrorContainsWhatString)
{
  ERR_raise(10, 11);
  auto ex = qi::ssl::Error::fromCurrentError("a custom error");
  EXPECT_THAT(ex.what(), testing::HasSubstr("a custom error"));
}

TEST(SslError, FromCurrentErrorClearsError)
{
  ERR_raise(10, 11);
  EXPECT_NE(0u, ::ERR_peek_error());
  auto ex = qi::ssl::Error::fromCurrentError("err");
  EXPECT_EQ(0u, ::ERR_peek_error());
}

TEST(SslError, FromCurrentErrorHasPreviousErrorCode)
{
  static const int libCode = ERR_get_next_error_library();
  ERR_raise(libCode, ERR_R_PASSED_INVALID_ARGUMENT);
  auto err = ::ERR_peek_error();
  ASSERT_EQ(libCode, ERR_GET_LIB(err));
  ASSERT_EQ(ERR_R_PASSED_INVALID_ARGUMENT, ERR_GET_REASON(err));
  auto ex = qi::ssl::Error::fromCurrentError("err");
  EXPECT_EQ(std::error_code(err, ssl::errorCategory()), ex.code());
}

namespace {
  struct MockSSLObject
  {
    MockSSLObject()
    {
      using namespace testing;
      // Using ON_CALL to increment/decrement the refcount allows us to override this behavior
      // where necessary in tests.
      ON_CALL(*this, upRef()).WillByDefault([this] { this->refcount++; return true; });
      ON_CALL(*this, free()).WillByDefault([this] { this->refcount--; });
    }
    MOCK_METHOD0(free, void());
    MOCK_METHOD0(upRef, bool());
    int refcount = 1;
  };
}

namespace qi { namespace ssl { namespace detail {

template<>
struct ObjectTraits<MockSSLObject>
{
  static inline void free(MockSSLObject* p) noexcept  { p->free(); }
  static inline bool upRef(MockSSLObject* p) noexcept { return p->upRef(); }
  static inline const char* name() noexcept  { return "MockSSLObject"; }
};

}}} // namespace qi::ssl::detail

namespace
{

struct MockObject : ssl::detail::Pointer<MockSSLObject>
{
  // Forward args to pointer constructor with no additional logic.
  template<typename... Args>
  explicit MockObject(Args&&... args) : Pointer(std::forward<Args>(args)...) {}

  MockObject(const MockObject& o) = default;
  MockObject(MockObject&& o) noexcept = default;

  MockObject& operator=(const MockObject& o) = default;
  MockObject& operator=(MockObject&& o) noexcept = default;
};

struct SslPointer : testing::Test
{
  // NiceMock: ignore uninteresting calls.
  testing::NiceMock<MockSSLObject> sslObj;
};

}

TEST_F(SslPointer, ConstructedOwned)
{
  EXPECT_EQ(1, sslObj.refcount);
  {
    // The reference count is not incremented.
    MockObject object(&sslObj, qi::ssl::Owned::True);
    EXPECT_EQ(1, sslObj.refcount);
  }
  EXPECT_EQ(0, sslObj.refcount);
}

TEST_F(SslPointer, ConstructedNotOwned)
{
  EXPECT_EQ(1, sslObj.refcount);
  {
    MockObject object(&sslObj, qi::ssl::Owned::False);
    // The reference count is incremented once.
    EXPECT_EQ(2, sslObj.refcount);
  }
  EXPECT_EQ(1, sslObj.refcount);
}

TEST_F(SslPointer, CopyIncreasesRefCount)
{
  EXPECT_EQ(1, sslObj.refcount);
  {
    MockObject object(&sslObj, qi::ssl::Owned::True);
    EXPECT_EQ(1, sslObj.refcount);
    {
      MockObject object2(object);
      EXPECT_EQ(2, sslObj.refcount);
    }
    EXPECT_EQ(1, sslObj.refcount);
  }
  EXPECT_EQ(0, sslObj.refcount);
}

TEST_F(SslPointer, CopyAssignmentIncreasesRefCount)
{
  EXPECT_EQ(1, sslObj.refcount);
  {
    MockObject object(&sslObj, qi::ssl::Owned::True);
    EXPECT_EQ(1, sslObj.refcount);
    {
      MockObject object2;
      EXPECT_EQ(1, sslObj.refcount);
      object2 = object;
      EXPECT_EQ(2, sslObj.refcount);
    }
    EXPECT_EQ(1, sslObj.refcount);
  }
  EXPECT_EQ(0, sslObj.refcount);
}

TEST_F(SslPointer, MoveDoesNotIncreaseRefCount)
{
  EXPECT_EQ(1, sslObj.refcount);
  {
    MockObject object(&sslObj, qi::ssl::Owned::True);
    EXPECT_EQ(1, sslObj.refcount);
    {
      MockObject object2(std::move(object));
      EXPECT_EQ(1, sslObj.refcount);
      EXPECT_EQ(nullptr, object);
    }
    EXPECT_EQ(0, sslObj.refcount);
  }
  EXPECT_EQ(0, sslObj.refcount);
}

TEST_F(SslPointer, MoveAssignmentDoesNotIncreaseRefCount)
{
  EXPECT_EQ(1, sslObj.refcount);
  {
    MockObject object(&sslObj, qi::ssl::Owned::True);
    EXPECT_EQ(1, sslObj.refcount);
    {
      MockObject object2;
      EXPECT_EQ(1, sslObj.refcount);
      object2 = std::move(object);
      EXPECT_EQ(1, sslObj.refcount);
      EXPECT_EQ(nullptr, object);
    }
    EXPECT_EQ(0, sslObj.refcount);
  }
  EXPECT_EQ(0, sslObj.refcount);
}

TEST_F(SslPointer, UpRefThrowsOnFailure)
{
  using namespace testing;
  EXPECT_EQ(1, sslObj.refcount);
  {
    MockObject object(&sslObj, qi::ssl::Owned::True);
    EXPECT_EQ(1, sslObj.refcount);

    // The object traits returns false on upRef, which means a failure.
    EXPECT_CALL(sslObj, upRef()).WillOnce(Return(false));
    EXPECT_THROW(object.upRef(), ssl::Error);
    EXPECT_EQ(1, sslObj.refcount);
  }
  // The SSL object is freed when `object` is destroyed.
  EXPECT_EQ(0, sslObj.refcount);
}

TEST(SslBIO, IsRegular)
{
  const char data[] = "muffins";
  EXPECT_TRUE(ka::is_regular({
        ssl::BIO{},
        ssl::BIO::fromRange(data)
  }));
}

TEST(SslBIOFromRange, ValidRange)
{
  const std::string data = "cookies";
  auto bio = ssl::BIO::fromRange(data);
  const auto bioData = bio.memoryData();
  // Do not compare `data` with `bioData.data()`, the second is not guaranteed to be null
  // terminated, which can result in a buffer overflow.
  EXPECT_EQ(data, std::string(bioData.begin(), bioData.end()));
}

TEST(SslBIOFromRange, InvalidRange)
{
  const std::string data = "cheesecake";
  // begin is not accessible from end.
  auto bio = ssl::BIO::fromRange(data.end(), data.begin());
  EXPECT_TRUE(bio.memoryData().empty());
}

template<typename It>
using FromRangeRes = decltype(ssl::BIO::fromRange(std::declval<It>(), std::declval<It>()));

TEST(SslBIOFromRange, OnlyValidForRAIter)
{
  using InputIt = std::istream_iterator<char>;
  using ForwardIt = std::forward_list<char>::iterator;
  using BidirectionalIt = std::list<char>::iterator;
  using RAIt = std::vector<char>::iterator;
  static_assert(std::is_same<FromRangeRes<InputIt>,         void>::value, "");
  static_assert(std::is_same<FromRangeRes<ForwardIt>,       void>::value, "");
  static_assert(std::is_same<FromRangeRes<BidirectionalIt>, void>::value, "");
  static_assert(std::is_same<FromRangeRes<RAIt>,            ssl::BIO>::value, "");
  SUCCEED();
}

TEST(SslBIOFromFile, CanRead)
{
  const std::string data = "cheesecake";
  const auto filename = writeTmpFile(data);
  auto bio = ssl::BIO::fromFile(filename);
  const auto bioData = bio.read(static_cast<int>(data.size()));
  // Do not compare `data` with `bioData.data()`, the second is not guaranteed to be null
  // terminated, which can result in a buffer overflow.
  EXPECT_EQ(data, std::string(bioData.begin(), bioData.end()));
}

TEST(SslBIOFromFile, CannotReadWithWriteMode)
{
  const auto filename = writeTmpFile("data");
  auto bio = ssl::BIO::fromFile(filename, "w");
  EXPECT_THROW(bio.read(1), ssl::Error);
}

TEST(SslBIOFromFile, BadMode)
{
  const auto filename = writeTmpFile("data");
  EXPECT_THROW(ssl::BIO::fromFile(filename, "bad mode"), ssl::Error);
}

TEST(SslBIOFromFile, NonExistingFile)
{
  EXPECT_THROW(ssl::BIO::fromFile("/file that is unlikely to exist"), ssl::Error);
}

TEST(SslBIOReadPrivateKey, KeyData)
{
  auto bio = ssl::BIO::fromRange(keyData);
  auto pkey = bio.readPemPrivateKey();
  EXPECT_TRUE(pkey);
}

TEST(SslBIOReadPrivateKey, InvalidKeyData)
{
  auto bio = ssl::BIO::fromRange(invalidKeyData);
  EXPECT_THROW(bio.readPemPrivateKey(), ssl::Error);
}

TEST(SslBIOReadPrivateKey, IgnoresOtherHeaders)
{
  auto bio = ssl::BIO::fromRange(cert1Data);
  auto pkey = bio.readPemPrivateKey();
  EXPECT_FALSE(pkey);
}

TEST(SslBIOReadPrivateKey, WithPassphrase)
{
  const std::string data = encryptedKeyData;
  auto bio = ssl::BIO::fromRange(data);
  auto cb = ssl::PemPasswordCallback::fromPassphrase(passphrase);
  auto pkey = bio.readPemPrivateKey(cb);
  EXPECT_TRUE(pkey);
}

TEST(SslBIOReadPrivateKey, WithPasswordCallback)
{
  const std::string data = encryptedKeyData;
  auto bio = ssl::BIO::fromRange(data);
  auto cbImpl = [](char* buf, int size, int rwflag, void* user) -> int {
    const auto passphrase = reinterpret_cast<const char*>(user);
    const auto passphraseLen = static_cast<int>(std::strlen(passphrase));
    EXPECT_GE(size, 0);
    EXPECT_GE(size, passphraseLen) << passphrase;
    if (size < passphraseLen)
      return -1;
    EXPECT_EQ(0, rwflag);
    std::copy_n(passphrase, passphraseLen, buf);
    return passphraseLen;
  };
  auto cb = ssl::PemPasswordCallback{ cbImpl, const_cast<char*>(passphrase) };
  auto pkey = bio.readPemPrivateKey(cb);
  EXPECT_TRUE(pkey);
}

TEST(SslBIOReadPrivateKey, WithWrongPassphraseThrows)
{
  const std::string data = encryptedKeyData;
  auto bio = ssl::BIO::fromRange(data);
  auto cb = ssl::PemPasswordCallback::fromPassphrase("probably wrong passphrase");
  EXPECT_THROW(bio.readPemPrivateKey(cb), ssl::Error);
}

/// Returns true if the two certificates have the same content.
static bool certEq(const ssl::Certificate& a, const ssl::Certificate& b)
{
  return a.cmp(b) == 0;
}

TEST(SslBIOReadPemX509, Cert1Data)
{
  auto bio = ssl::BIO::fromRange(cert1Data);
  auto cert = bio.readPemX509();
  EXPECT_TRUE(cert);
}

TEST(SslBIOReadPemX509, Cert2Data)
{
  auto bio = ssl::BIO::fromRange(cert2Data);
  auto cert = bio.readPemX509();
  EXPECT_TRUE(cert);
}

TEST(SslBIOReadPemX509, IgnoresOtherHeaders)
{
  auto bio = ssl::BIO::fromRange(keyData);
  auto cert = bio.readPemX509();
  EXPECT_FALSE(cert);
}

TEST(SslBIOReadPemX509, InvalidThrows)
{
  auto bio = ssl::BIO::fromRange(invalidCertData);
  EXPECT_THROW(bio.readPemX509(), ssl::Error);
}

TEST(SslBIOReadPemX509, Cert1Cert2)
{
  const std::string data =
    cert1Data
    + cert2Data;
  auto bio = ssl::BIO::fromRange(data);
  auto certs = bio.readPemX509Chain();

  // Certificates are expected to be the same as if they were read separately.
  std::vector<ssl::Certificate> expected = {
      *ssl::BIO::fromRange(cert1Data).readPemX509(),
      *ssl::BIO::fromRange(cert2Data).readPemX509(),
    };
  ASSERT_EQ(expected.size(), certs.size());
  const auto areEqual = std::equal(expected.begin(), expected.end(), certs.begin(), certEq);
  EXPECT_TRUE(areEqual);
}

TEST(SslBIOReadPemX509Chain, InvalidThrows)
{
  const std::string data =
    cert1Data
    + invalidCertData
    + cert2Data;
  auto bio = ssl::BIO::fromRange(data);
  EXPECT_THROW(bio.readPemX509Chain(), ssl::Error);
}

TEST(SslBIOReadPemX509Chain, IgnoresOtherHeaders)
{
  const std::string data =
    cert1Data
    + keyData
    + cert2Data;
  auto bio = ssl::BIO::fromRange(data);
  auto certs = bio.readPemX509Chain();

  std::vector<ssl::Certificate> expected = {
      *ssl::BIO::fromRange(cert1Data).readPemX509(),
      *ssl::BIO::fromRange(cert2Data).readPemX509(),
    };

  ASSERT_EQ(expected.size(), certs.size());
  const auto areEqual = std::equal(expected.begin(), expected.end(), certs.begin(), certEq);
  EXPECT_TRUE(areEqual);
}

TEST(SslPKey, IsRegular)
{
  EXPECT_TRUE(ka::is_regular({
        ssl::PKey(),
        *ssl::BIO::fromRange(keyData).readPemPrivateKey(),
  }));
}

TEST(SslPKeyPrivateFromFile, Success)
{
  const auto filename = writeTmpFile(keyData);
  const auto pkey = ssl::PKey::privateFromPemFile(filename);
  EXPECT_TRUE(pkey);
}

TEST(SslPKeyPrivateFromFile, Encrypted)
{
  const auto filename = writeTmpFile(encryptedKeyData);
  const auto pkey =
    ssl::PKey::privateFromPemFile(filename, "r", ssl::PemPasswordCallback::fromPassphrase(passphrase));
  EXPECT_TRUE(pkey);
}

TEST(SslPKeyPrivateFromFile, InvalidData)
{
  const auto filename = writeTmpFile(invalidKeyData);
  EXPECT_THROW(ssl::PKey::privateFromPemFile(filename), ssl::Error);
}

TEST(SslPKeyPrivateFromFile, NoKey)
{
  const auto filename = writeTmpFile(cert1Data);
  const auto pkey = ssl::PKey::privateFromPemFile(filename);
  EXPECT_FALSE(pkey);
}

TEST(SslPKeyPrivateFromFile, IgnoresOtherHeaders)
{
  const auto filename = writeTmpFile(cert1Data + keyData);
  const auto pkey = ssl::PKey::privateFromPemFile(filename);
  EXPECT_TRUE(pkey);
}

TEST(SslPKeyPrivateFromFile, NonExisting)
{
  EXPECT_THROW(ssl::PKey::privateFromPemFile("/file that is unlikely to exist"), ssl::Error);
}

TEST(SslPKeyPrivateFromRange, Success)
{
  const auto pkey = ssl::PKey::privateFromPemRange(keyData);
  EXPECT_TRUE(pkey);
}

TEST(SslPKeyPrivateFromRange, Encrypted)
{
  const auto pkey =
    ssl::PKey::privateFromPemRange(encryptedKeyData, ssl::PemPasswordCallback::fromPassphrase(passphrase));
  EXPECT_TRUE(pkey);
}

TEST(SslPKeyPrivateFromRange, NoKey)
{
  const auto pkey = ssl::PKey::privateFromPemRange(cert1Data);
  EXPECT_FALSE(pkey);
}

TEST(SslPKeyPrivateFromRange, IgnoresOtherHeaders)
{
  const auto pkey = ssl::PKey::privateFromPemRange(cert1Data + keyData);
  EXPECT_TRUE(pkey);
}

TEST(SslCertificate, IsRegular)
{
  EXPECT_TRUE(ka::is_regular({
    ssl::Certificate{},
    *ssl::Certificate::fromPemRange(cert1Data),
    *ssl::Certificate::fromPemRange(cert2Data),
  }));
}

TEST(SslCertificateFromFile, Success)
{
  const auto filename = writeTmpFile(cert1Data);
  const auto cert = ssl::Certificate::fromPemFile(filename);
  EXPECT_TRUE(cert);
}

TEST(SslCertificateFromFile, InvalidData)
{
  const auto filename = writeTmpFile(invalidCertData);
  EXPECT_THROW(ssl::Certificate::fromPemFile(filename), ssl::Error);
}

TEST(SslCertificateFromFile, NoCert)
{
  const auto filename = writeTmpFile(keyData);
  const auto cert = ssl::Certificate::fromPemFile(filename);
  EXPECT_FALSE(cert);
}

TEST(SslCertificateFromFile, IgnoresOtherHeaders)
{
  const auto filename = writeTmpFile(keyData + cert1Data);
  const auto cert = ssl::Certificate::fromPemFile(filename);
  EXPECT_TRUE(cert);
}

TEST(SslCertificateFromFile, NonExisting)
{
  EXPECT_THROW(ssl::Certificate::fromPemFile("/file that is unlikely to exist"), ssl::Error);
}

TEST(SslCertificateFromRange, Success)
{
  const auto cert = ssl::Certificate::fromPemRange(cert1Data);
  EXPECT_TRUE(cert);
}

TEST(SslCertificateFromRange, SuccessTrusted)
{
  const auto cert = ssl::Certificate::fromPemRange(cert1Data, true);
  EXPECT_TRUE(cert);
}

TEST(SslCertificateFromRange, NoCert)
{
  const auto cert = ssl::Certificate::fromPemRange(keyData);
  EXPECT_FALSE(cert);
}

TEST(SslCertificateFromRange, IgnoresOtherHeaders)
{
  const auto cert = ssl::Certificate::fromPemRange(keyData + cert1Data);
  EXPECT_TRUE(cert);
}

TEST(SslCertificateChainFromFile, Success)
{
  const auto filename = writeTmpFile(cert1Data + cert2Data);
  const auto certs = ssl::Certificate::chainFromPemFile(filename);
  std::vector<ssl::Certificate> expected = {
      *ssl::Certificate::fromPemRange(cert1Data),
      *ssl::Certificate::fromPemRange(cert2Data),
    };

  ASSERT_EQ(expected.size(), certs.size());
  const auto areEqual = std::equal(expected.begin(), expected.end(), certs.begin(), certEq);
  EXPECT_TRUE(areEqual);
}

TEST(SslCertificateChainFromFile, InvalidData)
{
  const auto filename = writeTmpFile(cert2Data + invalidCertData);
  EXPECT_THROW(ssl::Certificate::chainFromPemFile(filename), ssl::Error);
}

TEST(SslCertificateChainFromFile, NoCert)
{
  const auto filename = writeTmpFile(keyData);
  const auto certs = ssl::Certificate::chainFromPemFile(filename);
  EXPECT_TRUE(certs.empty());
}

TEST(SslCertificateChainFromFile, IgnoresOtherHeaders)
{
  const auto filename = writeTmpFile(cert1Data + keyData + cert2Data);
  const auto certs = ssl::Certificate::chainFromPemFile(filename);
  std::vector<ssl::Certificate> expected = {
      *ssl::Certificate::fromPemRange(cert1Data),
      *ssl::Certificate::fromPemRange(cert2Data),
    };

  ASSERT_EQ(expected.size(), certs.size());
  const auto areEqual = std::equal(expected.begin(), expected.end(), certs.begin(), certEq);
  EXPECT_TRUE(areEqual);
}

TEST(SslCertificateChainFromFile, NonExisting)
{
  EXPECT_THROW(ssl::Certificate::chainFromPemFile("/file that is unlikely to exist"), ssl::Error);
}

TEST(SslCertificateChainFromRange, Success)
{
  const auto certs = ssl::Certificate::chainFromPemRange(cert2Data + cert1Data);
  std::vector<ssl::Certificate> expected = {
      *ssl::Certificate::fromPemRange(cert2Data),
      *ssl::Certificate::fromPemRange(cert1Data),
    };

  ASSERT_EQ(expected.size(), certs.size());
  const auto areEqual = std::equal(expected.begin(), expected.end(), certs.begin(), certEq);
  EXPECT_TRUE(areEqual);
}

TEST(SslCertificateChainFromRange, NoCert)
{
  const auto certs = ssl::Certificate::chainFromPemRange(keyData + keyData);
  EXPECT_TRUE(certs.empty());
}

TEST(SslCertificateChainFromRange, IgnoresOtherHeaders)
{
  const auto certs = ssl::Certificate::chainFromPemRange(keyData + cert2Data + keyData + cert1Data + keyData);
  std::vector<ssl::Certificate> expected = {
      *ssl::Certificate::fromPemRange(cert2Data),
      *ssl::Certificate::fromPemRange(cert1Data),
    };

  ASSERT_EQ(expected.size(), certs.size());
  const auto areEqual = std::equal(expected.begin(), expected.end(), certs.begin(), certEq);
  EXPECT_TRUE(areEqual);
}

TEST(SslCertificateStore, IsRegular)
{
  EXPECT_TRUE(ka::is_regular({
        ssl::CertificateStore{},
        ssl::CertificateStore(X509_STORE_new()),
  }));
}
