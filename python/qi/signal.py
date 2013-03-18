'''Signal class provides a way to create and subscribe to signals.'''

import _qipyessagingswig

import _qipyessaging.application as _qipya

class Signal:
    def __init__(self):
        if not _qipya.Application.initialized():
            raise _qipya.MissingApplicationError(feature='Signals')
        self._obj = _qipyessagingswig.qi_signal()

    def connect(self, callback):
        '''Connects callback to the signal. When signal will be triggered,
        callback will be called with trigger's parameters.

        :param callback: Callback function (must be callable).
        :return: An identifying number needed to disconnect callback to signal.
        '''
        return self._obj.connect(callback)

    def disconnect(self, idx):
        '''Disconnect the callback identified with index *idx* to the signal.

        :param idx: Id identifying callback (returned by connect).
        '''
        return self._obj.disconnect(idx)

    def disconnectAll(self):
        '''Disconnect all callbacks.'''
        return self._obj.disconnectAll()

    def trigger(self, *args):
        '''Trigger signal with given arguments.'''
        return self._obj.trigger(*args)
