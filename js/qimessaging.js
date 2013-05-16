function QiSession(url)
{
  var _socket = io.connect(url);
  var _dfd = new Array();
  var _sigs = new Array();
  var _idm = 0;

  _socket.on('reply', function (data) {
    if (_dfd[data["idm"]].__service != undefined)
    {
      var o = new Object();
      var s = data["result"][0]
      o.__mobj = data["result"];

      var f = data["result"][1][0];
      for (var i in f)
      {
        var m = f[i][2];
        o[m] = createMetaCall(s, m);
      }

      _sigs[s] = new Array();
      var es = data["result"][1][1];
      for (var i in es)
      {
        var e = es[i][1];
        o[e] = createMetaSignal(s, e)
      }

      _dfd[data["idm"]].resolve(o);
    }
    else
    {
      _dfd[data["idm"]].resolve(data["result"]);
    }
    delete _dfd[data["idm"]];
  });

  _socket.on('error', function (data) {
    _dfd[data["idm"]].reject(data["result"]);
    delete _dfd[data["idm"]];
  });

  _socket.on('event', function (data) {
    var res = data["result"]
    var cbs = _sigs[res["service"]][res["signal"]]
    for (var i in cbs)
    {
      cbs[i](res["data"])
    }
  });

  function createMetaCall(service, method)
  {
    return function() {
      var idm = ++_idm;
      _dfd[idm] = $.Deferred();
      if (service == "ServiceDirectory" && method == "service")
      {
        _dfd[idm].__service = 1;
      }
      _socket.emit('call', { idm: idm, params: { service: service, method: method, args: Array.prototype.slice.call(arguments, 0) } });

      return _dfd[idm];
    }
  }

  function createMetaSignal(service, signal)
  {
    var e = new Object();
    _sigs[service][signal] = new Array();
    e.connect = function(cb) {
      var i = _sigs[service][signal].push(cb);
      return createMetaCall(service, "registerEvent")(signal, i);
    }
    e.disconnect = function(i) {
      delete _sigs[service][signal][i];
      return createMetaCall(service, "unregisterEvent")(signal, i)
    }
    return e;
  }

  this.service = createMetaCall("ServiceDirectory", "service");

  this.socket = function()
  {
    return _socket;
  }
}
