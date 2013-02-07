function QiSession(url)
{
  var _url = url;
  var _services = [];
  var _socket = io.connect(url);
  var _dfd = new Array();
  var _dfdD = new Array();
  var _idm = 0;

  var getIdm = (function() {
    var id = 0;
    return function() { return id++; };
  })();

  _socket.on('reply', function(data) {
    if (_dfdD[data["idm"]] != undefined)
    {
      service = _dfdD[data["idm"]]["service"]
      _services[service] = new Object();

      _services[service].__mobj = new Object();
      _services[service].__mobj.name = data["result"]["name"];
      _services[service].__mobj.doc = data["result"]["doc"];
      _services[service].__mobj.functions = new Array();

      for (i = 0; i < data["result"]["functions"].length; i++)
      {
        m = data["result"]["functions"][i];
        _services[service][m.name] = createMetaCall(_socket, service, m.name);
        _services[service].__mobj.functions[m.name] = m;
      }

      _dfd[data["idm"]].resolve(_services[service]);
      return;
    }

    _dfd[data["idm"]].resolve(data["result"]);
  });

  function createMetaCall(socket, service, method)
  {
    function metaCall()
    {
      var dfd = $.Deferred()

      var args = [];
      for (j = 0; j < arguments.length; j++)
      {
        args.push(arguments[j]);
      }

      var idm = getIdm();
      _socket.emit('call', { idm: idm, params: { service: service, method: method, args: args } });
      _dfd[idm] = dfd;

      return dfd;
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
    dfd = $.Deferred();

    if (_services[service] == undefined)
    {
      var idm = getIdm();
      _dfd[idm] = dfd;
      _dfdD[idm] = { service: service };
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
