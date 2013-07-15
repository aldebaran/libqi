/*
**  Copyright (C) Aldebaran Robotics
**  See COPYING for the license
**
**  Author(s):
**   - Laurent LEC    <llec@aldebaran-robotics.com>
**
**  QiMessaging master release
*/
function QiSession(url, resource)
{
  var _socket = io.connect(url, { resource: resource == undefined ? "socket.io" : resource });
  var _dfd = new Array();
  var _sigs = new Array();
  var _sigIdxToLink = new Array();
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
    else if (_dfd[data["idm"]].__event != undefined)
    {
      _sigIdxToLink[data["result"]] = _dfd[data["idm"]].__event;
      _dfd[data["idm"]].resolve(data["result"]);
    }
    else
    {
      _dfd[data["idm"]].resolve(data["result"]);
    }
    delete _dfd[data["idm"]];
  });

  _socket.on('error', function (data) {
    if (data["idm"] != undefined)
    {
      _dfd[data["idm"]].reject(data["result"]);
      delete _dfd[data["idm"]];
    }
  });

  _socket.on('event', function (data) {
    var res = data["result"]
    var cbs = _sigs[res["service"]][res["signal"]]
    for (var i in cbs)
    {
      cbs[i](res["data"])
    }
  });

  _socket.on('disconnect', function(data) {
    for (var idm in _dfd)
    {
      _dfd[idm].reject("Call " + idm + " canceled: disconnected");
      delete _dfd[idm];
    }
  });

  function createMetaCall(service, method, data)
  {
    return function() {
      var idm = ++_idm;
      _dfd[idm] = $.Deferred();
      if (service == "ServiceDirectory" && method == "service")
      {
        _dfd[idm].__service = 1;
      }
      else if (method == "registerEvent")
      {
        _dfd[idm].__event = data;
      }
      _socket.emit('call', { idm: idm, params: { service: service, method: method, args: Array.prototype.slice.call(arguments, 0) } });

      return _dfd[idm].promise();
    }
  }

  function createMetaSignal(service, signal)
  {
    var e = new Object();
    _sigs[service][signal] = new Array();
    e.connect = function(cb) {
      var i = _sigs[service][signal].push(cb) - 1;
      return createMetaCall(service, "registerEvent", i)(signal);
    }
    e.disconnect = function(l) {
      delete _sigs[service][signal][_sigIdxToLink[l]];
      delete _sigIdxToLink[l];
      return createMetaCall(service, "unregisterEvent")(signal, l);
    }
    return e;
  }

  this.service = createMetaCall("ServiceDirectory", "service");

  this.socket = function()
  {
    return _socket;
  }
}
