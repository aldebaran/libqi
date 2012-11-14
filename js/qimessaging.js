function qiLogError(str)
{
  console.log("failed: " + str);
}

function createMetaCall(url, service, method)
{
  function metaCall()
  {
    var dfd = $.Deferred()

    var args = [];
    for (j = 0; j < arguments.length; j++)
    {
      args.push(arguments[j]);
    }

    var params = { service: service, method: method, args: args };
    $.post(url + '/', params, function(data) {
      dfd.resolve(data["result"]) ;
    });

    return dfd;
  }
  return metaCall;
}

function QiSession(url)
{
  var _url = url;
  var _services = [];

  this.services = function()
  {
    var dfd = $.Deferred();

    var params = { service: "serviceDirectory", method: "services" };
    $.post(_url + '/', params, function(data) {
      _services = data["result"];
      dfd.resolve(_services);
    });

    return dfd;
  }

  this.service = function(service)
  {
    var dfd = $.Deferred();

    if (_services[service] === undefined)
    {
      var params = { service: "serviceDirectory", method: "service", args: [ service ] };
      $.post(_url + '/', params, function(data) {
        _services[service] = new Object();

        for (i = 0; i < data["result"].length; i++)
        {
          method = data["result"][i];
          _services[service][method] = createMetaCall(_url, service, method);
        }

        _services[service]._name = service;

        dfd.resolve(_services[service]);
      });
    }
    else
    {
      dfd.resolve(_services[service]);
    }

    return dfd;
  }
}
