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
    var idm = data["idm"];

    if (data["result"] != null && data["result"]["metaobject"] != undefined)
    {
      var o = new Object();
      o.__MetaObject = data["result"]["metaobject"];
      o.__MetaObject.pyobj = data["result"]["pyobject"];
      o.__MetaObject.name = _dfd[idm].__service;
      _sigs[o.__MetaObject.pyobj] = new Array();

      var methods = o.__MetaObject["methods"];
      for (var i in methods)
      {
        var methodName = methods[i]["name"];
        o[methodName] = createMetaCall(o.__MetaObject.pyobj, methodName);
      }

      var signals = o.__MetaObject["signals"];
      for (var i in signals)
      {
        var signalName = signals[i]["name"]
        o[signalName] = createMetaSignal(o.__MetaObject.pyobj, signalName);
      }

      _dfd[idm].resolve(o);
    }
    else if (_dfd[idm].__signal != undefined)
    {
      _sigIdxToLink[data["result"]] = _dfd[idm].__signal;
      _dfd[idm].resolve(data["result"]);
    }
    else
    {
      _dfd[idm].resolve(data["result"]);
    }
    delete _dfd[idm];
  });

  _socket.on('error', function (data) {
    if (data["idm"] != undefined)
    {
      _dfd[data["idm"]].reject(data["result"]);
      delete _dfd[data["idm"]];
    }
  });

  _socket.on('signal', function (data) {
    var res = data["result"];
    var callbacks = _sigs[res["obj"]][res["signal"]];
    for (var i in callbacks)
    {
      callbacks[i](res["data"])
    }
  });

  _socket.on('disconnect', function(data) {
    for (var idm in _dfd)
    {
      _dfd[idm].reject("Call " + idm + " canceled: disconnected");
      delete _dfd[idm];
    }
  });

  function createMetaCall(obj, method, data)
  {
    return function() {
      var idm = ++_idm;
      var args = Array.prototype.slice.call(arguments, 0);
      _dfd[idm] = $.Deferred();
      if (obj == "ServiceDirectory" && method == "service")
      {
        _dfd[idm].__service = args[0];
      }
      else if (method == "registerEvent")
      {
        _dfd[idm].__signal = data;
      }
      _socket.emit('call', { idm: idm, params: { obj: obj, method: method, args: args } });

      return _dfd[idm].promise();
    }
  }

  function createMetaSignal(obj, signal)
  {
    var e = new Object();
    _sigs[obj][signal] = new Array();
    e.connect = function(cb) {
      var i = _sigs[obj][signal].push(cb) - 1;
      return createMetaCall(obj, "registerEvent", i)(signal);
    }
    e.disconnect = function(l) {
      delete _sigs[obj][signal][_sigIdxToLink[l]];
      delete _sigIdxToLink[l];
      return createMetaCall(obj, "unregisterEvent")(signal, l);
    }
    return e;
  }

  this.service = createMetaCall("ServiceDirectory", "service");

  this.socket = function()
  {
    return _socket;
  }
}
