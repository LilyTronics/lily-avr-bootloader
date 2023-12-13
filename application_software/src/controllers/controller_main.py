"""
Main controller.
"""

import os
import time
import wx

from src.models.application_settings import ApplicationSettings
from src.models.boot_loader import BootLoader
from src.models.hex_file import write_hex_file
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

        filename = self._application_settings.get_flash_filename()
        if os.path.isfile(filename):
            self._view.set_flash_filename(filename)
        self._view.set_flash_verify(self._application_settings.get_flash_verify())

        self._view.Bind(wx.EVT_BUTTON, self._on_connect, id=self._view.ID_BUTTON_CONNECT)
        self._view.Bind(wx.EVT_BUTTON, self._on_flash_read, id=self._view.ID_BUTTON_READ)
        self._view.Bind(wx.EVT_CLOSE, self._on_view_close)

    ###########
    # Private #
    ###########

    def _connect(self):
        self._boot_loader = None
        self._view.set_version_label('')
        self._view.set_device_name_label('')
        self._view.set_module_name_label('')
        self._view.set_flash_size_label(0, 0)
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

    def _read_flash(self):
        title = 'Read FLASH'

        with wx.FileDialog(self._view, title, style=wx.FD_SAVE) as dlg:
            if dlg.ShowModal() == wx.ID_OK:
                filename = dlg.GetPath()
                if not filename.endswith('.hex'):
                    filename += '.hex'

                if self._boot_loader is None:
                    self._connect()
                if self._boot_loader is None:
                    return

                flash_size = self._boot_loader.get_flash_size() * 1024
                page_size = self._boot_loader.get_page_size()
                n_pages = int(flash_size / page_size)

                address = 0
                flash_data = b''
                self._view.set_progress(0, n_pages)
                try:
                    for i in range(1, n_pages + 1):
                        self._view.set_status_label('Reading page {} of {}'.format(i, n_pages))
                        self._boot_loader.set_page_address(address)
                        flash_data += self._boot_loader.read_page()
                        address += page_size
                        self._view.set_progress(i)
                        i += 1
                except Exception as e:
                    message = 'Error reading data from FLASH:\n{}'.format(e)
                    with wx.MessageDialog(self._view, message, title, wx.ICON_EXCLAMATION) as dlg:
                        dlg.ShowModal()
                    self._view.set_progress(0)
                    self._view.set_status_label('Error reading FLASH')
                    return

                try:
                    write_hex_file(filename, flash_data, 0)
                    if len(filename) > 70:
                        parts = filename.split(os.sep)
                        first_part = os.sep.join(parts[0:2]) + os.sep
                        last_part = ''
                        i = 2
                        while i < len(parts):
                            last_part = os.sep + os.sep.join(parts[i:])
                            if len(last_part) + len(first_part) + 3 < 70:
                                break
                            i += 1
                        filename = '{}...{}'.format(first_part, last_part)
                    self._view.set_status_label("Data written to: '{}'".format(filename))
                except Exception as e:
                    message = 'Could not write the data to the file:\n{}\n{}'.format(filename, e)
                    with wx.MessageDialog(self._view, message, title, wx.ICON_EXCLAMATION) as dlg:
                        dlg.ShowModal()
                    self._view.set_status_label('Error writing data to file')
                self._view.set_progress(0)

    ##################
    # Event handlers #
    ##################

    def _on_connect(self, event):
        wx.CallAfter(self._connect)
        event.Skip()

    def _on_flash_read(self, event):
        wx.CallAfter(self._read_flash)
        event.Skip()

    def _on_view_close(self, event):
        self._application_settings.store_interface_port(self._view.get_selected_port())
        self._application_settings.store_interface_speed(self._view.get_selected_speed())
        filename = self._view.get_flash_filename()
        if os.path.isfile(filename):
            self._application_settings.store_flash_filename(filename)
        self._application_settings.store_flash_verify(self._view.get_flash_verify())

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
