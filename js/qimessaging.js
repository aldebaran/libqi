function QiSession(url)
{
  var _services = [];
  var _socket = io.connect(url);
  var _dfd = new Array();
  var _dfdD = new Array();

  var getIdm = (function() {
    var id = 0;
    return function() { return id++; };
  })();

  _socket.on('reply', function(data) {
    if (_dfdD[data["idm"]] != undefined)
    {
      var service = _dfdD[data["idm"]]
      _dfdD[data["idm"]] = 0;
      _services[service] = new Object();

      _services[service].__mobj = new Object();
      _services[service].__mobj.name = data["result"]["name"];
      _services[service].__mobj.doc = data["result"]["doc"];
      _services[service].__mobj.functions = new Array();

      for (var i = 0; i < data["result"]["functions"].length; ++i)
      {
        var m = data["result"]["functions"][i];
        _services[service][m.name] = createMetaCall(service, m.name);
        _services[service].__mobj.functions[m.name] = m;
      }

      _dfd[data["idm"]].resolve(_services[service]);
    }
    else
    {
      _dfd[data["idm"]].resolve(data["result"]);
    }

    _dfd[data["idm"]] = 0;
  });

  _socket.on('error', function(data) {
    _dfd[data["idm"]].reject(data["result"]);
    _dfd[data["idm"]] = 0;
  });

  function createMetaCall(service, method)
  {
    function metaCall()
    {
      var idm = getIdm();
      _dfd[idm] = $.Deferred();
      _socket.emit('call', { idm: idm, params: { service: service, method: method, args: Array.prototype.slice.call(arguments, 0) } });

      return _dfd[idm];
    }

    return metaCall;
  }

  this.services = function()
  {
    var idm = getIdm();
    _dfd[idm] = $.Deferred();
    _socket.emit('call', { idm: idm, params: { service: "serviceDirectory", method: "services" } });

    return _dfd[idm];
  }

  this.service = function(service)
  {
    var dfd = $.Deferred();

    if (_services[service] == undefined)
    {
      var idm = getIdm();
      _dfd[idm] = dfd;
      _dfdD[idm] = service;
      _socket.emit('call', { idm: idm, params: { service: "serviceDirectory", method: "service", args: [ service ] } });
    }
    else
    {
      dfd.resolve(_services[service])
    }

    return dfd;
  }

  this.socket = function()
  {
    return _socket;
  }
}
