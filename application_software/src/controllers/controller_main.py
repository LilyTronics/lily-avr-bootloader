"""
Main controller.
"""

import time
import wx

from src.models.application_settings import ApplicationSettings
from src.models.boot_loader import BootLoader
from src.models.list_serial_ports import get_available_serial_ports
from src.models.list_serial_ports import get_available_baud_rates
from src.views.view_main import ViewMain


class ControllerMain(object):

    def __init__(self, title):
        self._boot_loader = None
        self._application_settings = ApplicationSettings()
        self._view = ViewMain(title)
        pos = self._application_settings.get_main_window_position()
        if -1 not in pos:
            self._view.SetPosition(pos)

        ports = get_available_serial_ports()
        active_port = self._application_settings.get_interface_port()
        if active_port not in ports:
            active_port = ports[0]
        self._view.setup_list_of_serial_ports(ports, active_port)

        baudrates = get_available_baud_rates()
        speed = self._application_settings.get_interface_speed()
        if speed not in baudrates:
            speed = 19200
        self._view.setup_list_of_baudrates(baudrates, speed)

        self._view.Bind(wx.EVT_BUTTON, self._on_connect, id=self._view.ID_BUTTON_CONNECT)
        self._view.Bind(wx.EVT_CLOSE, self._on_view_close)

    ###########
    # Private #
    ###########

    def _connect(self):
        self._boot_loader = None
        try:
            self._boot_loader = BootLoader(self._view.get_selected_port(), self._view.get_selected_speed())
            time.sleep(0.5)
            if not self._boot_loader.activate():
                raise Exception('Could not activate the bootloader')
            self._view.set_version_label(self._boot_loader.get_version())
            self._view.set_device_name_label(self._boot_loader.get_device_name())
            self._view.set_module_name_label(self._boot_loader.get_module_name())
            self._view.set_flash_size_label(self._boot_loader.get_flash_size(), self._boot_loader.get_boot_size())
        except Exception as e:
            message = 'Could not connect to the bootloader:\n{}'.format(e)
            with wx.MessageDialog(self._view, message, 'Connect', wx.ICON_EXCLAMATION) as dlg:
                dlg.ShowModal()
            self._boot_loader = None

    ##################
    # Event handlers #
    ##################

    def _on_connect(self, event):
        wx.CallAfter(self._connect)
        event.Skip()

    def _on_view_close(self, event):
        self._application_settings.store_interface_port(self._view.get_selected_port())
        self._application_settings.store_interface_speed(self._view.get_selected_speed())

        if not self._view.IsIconized():
            self._application_settings.store_main_window_position(tuple(self._view.GetPosition()))

        event.Skip()

    ##########
    # Public #
    ##########

    def show_view(self):
        self._view.Show()


if __name__ == '__main__':

    app = wx.App(redirect=False)

    controller = ControllerMain('ControllerMain Test')
    controller.show_view()

    app.MainLoop()
