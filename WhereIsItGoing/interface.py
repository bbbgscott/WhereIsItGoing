#! /usr/bin/env python
# -*- python -*-

import os
import ttk
import threading
from pylab import *
from Tkinter import *
import geo_helper as geo
import sqlite_helper as sql
import TkTreectrl as treectrl


def vp_start_gui():
    '''Starting point when module is the main routine.'''
    if not os.path.exists("../db/super_spy.db"):
        sql.create_tcp_table()
    global val, w, root
    root = Tk()
    root.title('WhereIsItGoing')
    root.geometry('1200x600+339+204')
    w = TCPSuperSpy(root)
    init()
    root.mainloop()

w = None
t = None
last = 0


def create_TCPSuperSpy(root):
    '''Starting point when module is imported by another program.'''
    global w, w_win
    if w:  # So we have only one instance of window.
        return
    w = Toplevel(root)
    w.title('TCPSuperSpy')
    w.geometry('1200x600+339+204')
    w_win = TCPSuperSpy(w)
    init()
    return w_win


def destroy_TCPSuperSpy():
    global w
    kill_timer()
    w.destroy()
    w = None


def init():
    pass


def TODO():
        print ('TODO')


class TCPSuperSpy:
    def select_cmd(self, y):
        self.Geo_Info['state'] = 'normal'
        self.Geo_Info.delete('1.0', 'end')
        tup = self.mlb.get(y[0])
        ip = geo.find_ip(tup[0][3])
        if ip is None:
            self.Geo_Info.insert('end', 'Could not find info for ip address: ' + tup[0][3])
        else:
            self.Geo_Info.insert('end', 'Info for address: ' + tup[0][3] + '\n')
            self.Geo_Info.insert('end', 'Country: ' + str(ip['country_name']) + '\n')
            self.Geo_Info.insert('end', 'City: ' + ip['city'] + '\n')
        self.Geo_Info['state'] = 'disabled'

    def gen_graph(self):
        figure(1, figsize=(6, 6))
        axes([0.1, 0.1, 0.8, 0.8])
        mylabels = []
        fracs = []
        dict_ = {}

        dict_ = sql.get_countries()
        for value in dict_.values():
            fracs.append(value)
        for key in dict_.keys():
            mylabels.append(key)

        mycolors = ['red', 'blue', 'green', 'brown', 'cyan', 'black', 'magenta']
        pie(fracs, labels=mylabels, colors=mycolors)

        show()

    def write_report(self, table):
        self.Report_general['state'] = 'normal'
        self.Report_general.delete('1.0', 'end')
        for row in table:
            #self.Report_general.insert('end', row)
            self.Report_general.insert('end', str(row[0]) + ' ' + str(row[1]) + ' ' + str(row[2]) + ' ' + str(row[3]) + '\n')

        self.Report_general['state'] = 'disabled'

    def write_10_min(self, table):
        pass

    def __init__(self, master=None):
        # Set background of toplevel window to match
        # current style
        style = ttk.Style()
        theme = style.theme_use()
        default = style.lookup(theme, 'background')
        master.configure(background=default)

        self.menubar = Menu(master, font="font12", bg='#d9d9d9', fg='#000000')
        master.configure(menu=self.menubar)

        self.file = Menu(master, tearoff=0)
        self.menubar.add_cascade(menu=self.file,
                activebackground="#d9d9d9",
                activeforeground="#000000",
                background="#d9d9d9",
                font="font12",
                foreground="#000000",
                label="File")
        self.file.add_command(
                activebackground="#d9d9d9",
                activeforeground="#000000",
                background="#d9d9d9",
                command=TODO,
                font="font12",
                foreground="#000000",
                label="New")
        self.file.add_command(
                activebackground="#d9d9d9",
                activeforeground="#000000",
                background="#d9d9d9",
                command=TODO,
                font="font12",
                foreground="#000000",
                label="Open")
        self.file.add_command(
                activebackground="#d9d9d9",
                activeforeground="#000000",
                background="#d9d9d9",
                command=TODO,
                font="font12",
                foreground="#000000",
                label="Save")
        self.file.add_command(
                activebackground="#d9d9d9",
                activeforeground="#000000",
                background="#d9d9d9",
                command=TODO,
                font="font12",
                foreground="#000000",
                label="Save As...")
        self.file.add_command(
                activebackground="#d9d9d9",
                activeforeground="#000000",
                background="#d9d9d9",
                command=root.destroy,
                #command=root.quit,
                font="font12",
                foreground="#000000",
                label="Exit")
        self.edit = Menu(master, tearoff=0)
        self.menubar.add_cascade(menu=self.edit,
                activebackground="#d9d9d9",
                activeforeground="#000000",
                background="#d9d9d9",
                font="font12",
                foreground="#000000",
                label="Edit")
        self.edit.add_command(
                activebackground="#d9d9d9",
                activeforeground="#000000",
                background="#d9d9d9",
                command=TODO,
                font="font12",
                foreground="#000000",
                label="Cut")
        self.edit.add_command(
                activebackground="#d9d9d9",
                activeforeground="#000000",
                background="#d9d9d9",
                command=TODO,
                font="font12",
                foreground="#000000",
                label="Copy")
        self.edit.add_command(
                activebackground="#d9d9d9",
                activeforeground="#000000",
                background="#d9d9d9",
                command=TODO,
                font="font12",
                foreground="#000000",
                label="Paste")

        self.mlb = treectrl.MultiListbox(master)
        self.mlb.place(relx=0.0, rely=0.0, relheight=1.0, relwidth=0.55)
        self.mlb.configure(selectbackground="#c4c4c4")
        self.mlb.config(columns=('Unix Time',
                                'Local IP',
                                'Local Port',
                                'Remote IP',
                                'Remote Port',
                                'Connection Status',
                                'Executable'))
        # Timestamp, Local IP, Local Port, Remote IP, Connection Status
        self.mlb.configure(selectcmd=self.select_cmd, selectmode='single')
        self.update_table()
        """
        table = sql.read_table()
        for row in table:
            self.mlb.insert('end', *map(unicode, row[1:]))
        """

        """self.Outline = Frame(master)
        self.Outline.place(relx=0.56, rely=0.02, relheight=0.44, relwidth=0.42)
        self.Outline.configure(relief=GROOVE)
        self.Outline.configure(borderwidth='2')
        self.Outline.configure(relief='groove')"""

        self.Outline = ttk.Notebook(master)
        self.Outline.place(relx=0.56, rely=0.02, relheight=0.96, relwidth=0.42)

        self.Geo_Info = Text(self.Outline)
        self.Geo_Info.place(relx=0.03, rely=0.02, relheight=0.47, relwidth=0.95)
        self.Geo_Info.configure(background="#cccccc")
        self.Geo_Info.configure(wrap='none')
        self.Geo_Info.configure(state='disabled')

        self.Geo_image = Canvas(self.Outline)
        self.Geo_image.place(relx=0.03, rely=0.02, relheight=0.47, relwidth=0.95)
        self.Geo_image.configure(background="#ffffff")

        self.Report_10 = Text(self.Outline)
        self.Report_10.place(relx=0.03, rely=0.02, relheight=0.97, relwidth=0.95)
        self.Report_10.configure(background='#10ab8c')
        self.Report_10.configure(wrap='none')
        self.Report_10.configure(state='disabled')

        self.Report_hour = Text(self.Outline)
        self.Report_hour.place(relx=0.03, rely=0.02, relheight=0.97, relwidth=0.95)
        self.Report_hour.configure(background='#10ab8c')
        self.Report_hour.configure(wrap='none')
        self.Report_hour.configure(state='disabled')

        self.Report_day = Text(self.Outline)
        self.Report_day.place(relx=0.03, rely=0.02, relheight=0.97, relwidth=0.95)
        self.Report_day.configure(background='#10ab8c')
        self.Report_day.configure(wrap='none')
        self.Report_day.configure(state='disabled')

        self.Report_general = Text(self.Outline)
        self.Report_general.place(relx=0.03, rely=0.02, relheight=0.97, relwidth=0.95)
        self.Report_general.configure(background='#cccccc')
        self.Report_general.configure(wrap='none')
        self.Report_general.configure(state='disabled')

        # Pack the tabs
        self.Outline.add(self.Geo_Info, text='Geo Info')
        self.Outline.add(self.Geo_image, text='Pretty Graphs')
        self.Outline.add(self.Report_10, text='10 Min')
        self.Outline.add(self.Report_hour, text='hour')
        self.Outline.add(self.Report_day, text='day')
        self.Outline.add(self.Report_general, text='General Report')

        self.Outline.bind('<1>', self.on_click)

    def update_table(self):
        global t
        global last
        rowid = sql.get_latest_id()
        if t == None:
            t = threading.Timer(5.0, self.update_table).start()
        if rowid > last:
            last = rowid
            table = sql.read_table()
            for row in table:
                self.mlb.insert('end', *map(unicode, row[1:]))

    def kill_timer(self):
        global t
        t.exit()

    def on_click(self, event):
        if event.widget.identify(event.x, event.y) == 'label':
            index = event.widget.index('@%d, %d' % (event.x, event.y))
            #print event.widget.index(index)
            tabby = event.widget.tab(index)['text']
            if tabby == '10 Min':
                table = sql.report_10_min()
                self.write_10_min(table)
            elif tabby == 'hour':
                table = sql.report_hour()
            elif tabby == 'day':
                table = sql.report_day()
            elif tabby == 'Pretty Graphs':
                self.gen_graph()
            elif tabby == 'General Report':
                table = sql.general_report()
                self.write_report(table)


# The following code is added to facilitate the Scrolled widgets you specified.
class AutoScroll(object):
    '''Configure the scrollbars for a widget.'''

    def __init__(self, master):
        vsb = ttk.Scrollbar(master, orient='vertical', command=self.yview)
        hsb = ttk.Scrollbar(master, orient='horizontal', command=self.xview)

        self.configure(yscrollcommand=self._autoscroll(vsb),
            xscrollcommand=self._autoscroll(hsb))
        self.grid(column=0, row=0, sticky='nsew')
        vsb.grid(column=1, row=0, sticky='ns')
        hsb.grid(column=0, row=1, sticky='ew')

        master.grid_columnconfigure(0, weight=1)
        master.grid_rowconfigure(0, weight=1)

        # Copy geometry methods of master  (took from ScrolledText.py)
        if py31:
            methods = Pack.__dict__.keys() | Grid.__dict__.keys() \
                  | Place.__dict__.keys()
        else:
            methods = Pack.__dict__.keys() + Grid.__dict__.keys() \
                  + Place.__dict__.keys()

        for meth in methods:
            if meth[0] != '_' and meth not in ('config', 'configure'):
                setattr(self, meth, getattr(master, meth))

    @staticmethod
    def _autoscroll(sbar):
        '''Hide and show scrollbar as needed.'''
        def wrapped(first, last):
            first, last = float(first), float(last)
            if first <= 0 and last >= 1:
                sbar.grid_remove()
            else:
                sbar.grid()
            sbar.set(first, last)
        return wrapped

    def __str__(self):
        return str(self.master)


def _create_container(func):
    '''Creates a ttk Frame with a given master, and use this new frame to
    place the scrollbars and the widget.'''
    def wrapped(cls, master, **kw):
        container = ttk.Frame(master)
        return func(cls, container, **kw)
    return wrapped


class MultiListBox(AutoScroll, Listbox):
    '''A standard Tkinter Text widget with scrollbars that will
    automatically show/hide as needed.'''
    @_create_container
    def __init__(self, master, **kw):
        Listbox.__init__(self, master, **kw)
        AutoScroll.__init__(self, master)

if __name__ == '__main__':
    vp_start_gui()
