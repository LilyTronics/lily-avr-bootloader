"""
Main controller.
"""

import wx

from src.models.application_settings import ApplicationSettings
from src.models.list_serial_ports import get_available_serial_ports
from src.models.list_serial_ports import get_available_baud_rates
from src.views.view_main import ViewMain


class ControllerMain(object):

    def __init__(self, title):
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

        self._view.Bind(wx.EVT_CLOSE, self._on_view_close)

    ###################
    # Event handlsers #
    ###################

    def _on_view_close(self, event):
        self._application_settings.store_interface_port(self._view.get_selected_port())
        self._application_settings.store_interface_speed(self._view.get_selected_speed())

        if not self._view.IsIconized():
            self._application_settings.store_main_window_position(tuple(self._view.GetPosition()))

        event.Skip()

    def show_view(self):
        self._view.Show()


if __name__ == '__main__':

    app = wx.App(redirect=False)

    controller = ControllerMain('ControllerMain Test')
    controller.show_view()

    app.MainLoop()
