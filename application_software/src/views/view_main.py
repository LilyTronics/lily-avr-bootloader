"""
Main view for the application.
"""

import wx


class ViewMain(wx.Frame):

    _WINDOW_STYLE = wx.DEFAULT_FRAME_STYLE & ~(wx.RESIZE_BORDER | wx.MAXIMIZE_BOX)
    _INIT_SIZE = (600, 470)
    _SPACE = 10
    _LABEL_WIDTH = 100

    def __init__(self, title):
        super().__init__(None, wx.ID_ANY, title, style=self._WINDOW_STYLE)
        panel = wx.Panel(self, wx.ID_ANY)

        box = wx.BoxSizer(wx.VERTICAL)
        box.Add(self._create_interface_controls(panel), 0, wx.EXPAND | wx.ALL, self._SPACE)
        box.Add(self._create_info_controls(panel), 0, wx.EXPAND | wx.LEFT | wx.RIGHT, self._SPACE)
        box.Add(self._create_flash_controls(panel), 0, wx.EXPAND | wx.LEFT | wx.RIGHT, self._SPACE)

        panel.SetSizer(box)
        self.SetInitialSize(self._INIT_SIZE)

    def _create_interface_controls(self, parent):
        lbl_port = wx.StaticText(parent, wx.ID_ANY, 'Serial port:', size=(self._LABEL_WIDTH, -1))
        self._cmb_port = wx.ComboBox(parent, wx.ID_ANY, style=wx.CB_READONLY)

        lbl_speed = wx.StaticText(parent, wx.ID_ANY, 'Speed:', size=(self._LABEL_WIDTH, -1))
        self._cmb_speed = wx.ComboBox(parent, wx.ID_ANY, style=wx.CB_READONLY)

        btn_connect = wx.Button(parent, wx.ID_ANY, 'Connect')

        grid = wx.GridBagSizer(self._SPACE, self._SPACE)
        grid.Add(lbl_port, (0, 0), wx.DefaultSpan, wx.ALIGN_CENTER_VERTICAL)
        grid.Add(self._cmb_port, (0, 1), wx.DefaultSpan, wx.ALIGN_CENTER_VERTICAL)
        grid.Add(lbl_speed, (1, 0), wx.DefaultSpan, wx.ALIGN_CENTER_VERTICAL)
        grid.Add(self._cmb_speed, (1, 1), wx.DefaultSpan, wx.ALIGN_CENTER_VERTICAL)
        grid.Add(btn_connect, (2, 0), (1, 2), wx.ALIGN_CENTER_VERTICAL)

        box = wx.StaticBoxSizer(wx.StaticBox(parent, wx.ID_ANY, ' Interface settings: '), wx.VERTICAL)
        box.Add(grid, 1, wx.EXPAND | wx.ALL, self._SPACE)

        return box

    def _create_info_controls(self, parent):
        lbl_version = wx.StaticText(parent, wx.ID_ANY, 'Bootloader version:', size=(self._LABEL_WIDTH, -1))
        lbl_device_name = wx.StaticText(parent, wx.ID_ANY, 'Device name:', size=(self._LABEL_WIDTH, -1))
        lbl_module_name = wx.StaticText(parent, wx.ID_ANY, 'Module name:', size=(self._LABEL_WIDTH, -1))
        lbl_flash_size = wx.StaticText(parent, wx.ID_ANY, 'Flash size:')

        self._lbl_version = wx.StaticText(parent, wx.ID_ANY, '-')
        self._lbl_device_name = wx.StaticText(parent, wx.ID_ANY, '-')
        self._lbl_module_name = wx.StaticText(parent, wx.ID_ANY, '-')
        self._lbl_flash_size = wx.StaticText(parent, wx.ID_ANY, '-')

        grid = wx.GridBagSizer(self._SPACE, self._SPACE)
        grid.Add(lbl_version, (0, 0), wx.DefaultSpan, wx.ALIGN_CENTER_VERTICAL)
        grid.Add(self._lbl_version, (0, 1), wx.DefaultSpan, wx.ALIGN_CENTER_VERTICAL)
        grid.Add(lbl_device_name, (1, 0), wx.DefaultSpan, wx.ALIGN_CENTER_VERTICAL)
        grid.Add(self._lbl_device_name, (1, 1), wx.DefaultSpan, wx.ALIGN_CENTER_VERTICAL)
        grid.Add(lbl_module_name, (2, 0), wx.DefaultSpan, wx.ALIGN_CENTER_VERTICAL)
        grid.Add(self._lbl_module_name, (2, 1), wx.DefaultSpan, wx.ALIGN_CENTER_VERTICAL)
        grid.Add(lbl_flash_size, (3, 0), wx.DefaultSpan, wx.ALIGN_CENTER_VERTICAL)
        grid.Add(self._lbl_flash_size, (3, 1), wx.DefaultSpan, wx.ALIGN_CENTER_VERTICAL)

        box = wx.StaticBoxSizer(wx.StaticBox(parent, wx.ID_ANY, ' Information: '), wx.VERTICAL)
        box.Add(grid, 1, wx.EXPAND | wx.ALL, self._SPACE)

        return box

    def _create_flash_controls(self, parent):
        lbl_filename = wx.StaticText(parent, wx.ID_ANY, 'Filename:', size=(self._LABEL_WIDTH, -1))
        self._filename = wx.FilePickerCtrl(parent, wx.ID_ANY)

        lbl_verify = wx.StaticText(parent, wx.ID_ANY, 'Verify after write:', size=(self._LABEL_WIDTH, -1))
        self._chk_verify = wx.CheckBox(parent, wx.ID_ANY)

        grid = wx.GridBagSizer(self._SPACE, self._SPACE)
        grid.Add(lbl_filename, (0, 0), wx.DefaultSpan, wx.ALIGN_CENTER_VERTICAL)
        grid.Add(self._filename, (0, 1), wx.DefaultSpan, wx.ALIGN_CENTER_VERTICAL | wx.EXPAND)
        grid.Add(lbl_verify, (1, 0), wx.DefaultSpan, wx.ALIGN_CENTER_VERTICAL)
        grid.Add(self._chk_verify, (1, 1), wx.DefaultSpan, wx.ALIGN_CENTER_VERTICAL)
        grid.AddGrowableCol(1)

        btn_read = wx.Button(parent, wx.ID_ANY, 'Read')
        btn_write = wx.Button(parent, wx.ID_ANY, 'Write')

        buttons = wx.BoxSizer(wx.HORIZONTAL)
        buttons.Add(btn_read, 0, wx.ALL, self._SPACE)
        buttons.Add(btn_write, 0, wx.ALL, self._SPACE)

        box = wx.StaticBoxSizer(wx.StaticBox(parent, wx.ID_ANY, ' Flash programming: '), wx.VERTICAL)
        box.Add(grid, 1, wx.EXPAND | wx.ALL, self._SPACE)
        box.Add(buttons, 0, self._SPACE)

        return box


if __name__ == '__main__':

    app = wx.App(redirect=False)

    frame = ViewMain('ViewMain Test')
    frame.Show()

    app.MainLoop()
