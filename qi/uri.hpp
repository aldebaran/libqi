/*
**  Copyright (C) 2020 SoftBank Robotics Europe
**  See COPYING for the license
*/

#pragma once

#ifndef QI_URI_HPP
#define QI_URI_HPP

#include <qi/api.hpp>
#include <ka/uri.hpp>
#include <ka/uri/io.hpp>

namespace qi
{

/// True iff the host is a loopback address, i.e. `localhost`, `::1` or within `127.0.0.0/8`.
QI_API bool isLoopbackAddress(const std::string& host);

using Uri = ka::uri_t;
using UriAuthority = ka::uri_authority_t;
using UriUserinfo = ka::uri_userinfo_t;

using ka::username;
using ka::password;
using ka::userinfo;
using ka::host;
using ka::port;
using ka::scheme;
using ka::authority;
using ka::query;
using ka::fragment;
using ka::to_string;

// There is `path` namespace in qi so we cannot just import the `ka::path` symbol, we have to
// define a function with a different name.
inline std::string uriPath(const Uri& u) { return ka::path(u); }

/// Constructs a URI from a string.
///
/// @returns An empty optional if a URI could not be constructed from the entire input.
QI_API ka::opt_t<Uri> uri(const std::string& str) noexcept;

/// True iff the parameter is a URI with the `qi` scheme and with no authority.
QI_API bool isRelativeEndpoint(const Uri& uri);

/// The `qi` URI scheme.
QI_API const char* uriQiScheme();

/// Total ordering from most preferred to least preferred service endpoint (using PCRE syntax):
///   - qi:ServiceDirectory or qi:./ServiceDirectory
///   - qi:(?!//).*
///   - .*://loopback-address.*
///   - .*://.*
///   - .*
QI_API bool isPreferredEndpoint(const Uri& lhs, const Uri& rhs);

} // namespace qi

#endif // QI_URI_HPP
