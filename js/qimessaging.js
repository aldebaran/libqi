function QiSession(url)
{
  var _socket = io.connect(url);
  var _dfd = new Array();
  var _idm = 0;

  _socket.on('reply', function (data) {
    if (_dfd[data["idm"]].__service != undefined)
    {
      var o = new Object();
      o.__mobj = data["result"];

      var f = data["result"][1][0];
      for (var i in f)
      {
        var m = f[i][2];
        o[m] = createMetaCall(data["result"][0], m);
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

  this.service = createMetaCall("ServiceDirectory", "service");

  this.socket = function()
  {
    return _socket;
  }
}
