"""
Application settings model.
"""

import json
import os.path

from src.app_init import EXE_NAME
from src.app_init import USER_FOLDER


class ApplicationSettings(object):

    def __init__(self):
        self._filename = os.path.join(USER_FOLDER, '%s.json' % EXE_NAME)

    ###########
    # Private #
    ###########

    def _get_settings(self):
        try:
            return json.load(open(self._filename, 'r'))
        except (Exception,):
            pass

        return {}

    def _store_settings(self, settings):
        try:
            json.dump(settings, open(self._filename, 'w'), indent=2)
        except (Exception,):
            pass

    def _get_value(self, main_key, sub_key, default_value):
        value = default_value
        settings = self._get_settings()
        try:
            value = settings[main_key][sub_key]
        except KeyError:
            pass

        return value

    def _store_value(self, main_key, sub_key, value):
        settings = self._get_settings()
        if main_key not in settings.keys():
            settings[main_key] = {}
        settings[main_key][sub_key] = value
        self._store_settings(settings)

    #################################
    # Main window position and size #
    #################################

    def get_main_window_position(self): return self._get_value('main_window', 'position', [-1, -1])
    def store_main_window_position(self, value): self._store_value('main_window', 'position', value)
    def get_main_window_size(self): return self._get_value('main_window', 'size', [-1, -1])
    def store_main_window_size(self, value): self._store_value('main_window', 'size', value)
    def get_main_window_maximized(self): return self._get_value('main_window', 'maximized', False)
    def store_main_window_maximized(self, value): self._store_value('main_window', 'maximized', value)

    ######################
    # Interface settings #
    ######################

    def get_interface_port(self): return self._get_value('interface', 'port', '')
    def store_interface_port(self, value): self._store_value('interface', 'port', value)
    def get_interface_speed(self): return self._get_value('interface', 'speed', 115200)
    def store_interface_speed(self, value): self._store_value('interface', 'speed', value)

    ##################
    # Flash settings #
    ##################

    def get_flash_filename(self): return self._get_value('flash', 'filename', '')
    def store_flash_filename(self, value): self._store_value('flash', 'filename', value)

    def get_flash_verify(self): return self._get_value('flash', 'verify', True)
    def store_flash_verify(self, value): self._store_value('flash', 'verify', value)


if __name__ == '__main__':

    def _test_setting(label, get, store):
        print('\n{}:'.format(label))
        value = get()
        print(' - current value:', value)
        if type(value) is list:
            value[0] += 10
            value[1] += 10
        elif type(value) is bool:
            value = not value
        elif type(value) is str:
            value = 'spam' if value != 'spam' else 'ham'
        elif type(value) is int:
            value += 10
        else:
            raise Exception('Invalid type:', type(value))
        print(' - new value    :', value)
        store(value)
        stored_value = get()
        print(' - stored value :', stored_value)
        assert stored_value == value, 'Setting not stored correct'


    app_settings = ApplicationSettings()

    _test_setting('Main window position',
                  app_settings.get_main_window_position,
                  app_settings.store_main_window_position)

    _test_setting('Main window size',
                  app_settings.get_main_window_size,
                  app_settings.store_main_window_size)

    _test_setting('Main window maximized',
                  app_settings.get_main_window_maximized,
                  app_settings.store_main_window_maximized)

    _test_setting('Interface port',
                  app_settings.get_interface_port,
                  app_settings.store_interface_port)

    _test_setting('Interface speed',
                  app_settings.get_interface_speed,
                  app_settings.store_interface_speed)

    _test_setting('Flash filename',
                  app_settings.get_flash_filename,
                  app_settings.store_flash_filename)

    _test_setting('Flash verify',
                  app_settings.get_flash_verify,
                  app_settings.store_flash_verify)
