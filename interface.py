#! /usr/bin/env python
# -*- python -*-

import sys
from Tkinter import *
import ttk
import TkTreectrl as treectrl
import geo_helper
import sqlite_helper


def vp_start_gui():
    '''Starting point when module is the main routine.'''
    global val, w, root
    root = Tk()
    root.title('TCPSuperSpy')
    root.geometry('800x600+339+204')
    w = TCPSuperSpy(root)
    init()
    root.mainloop()

w = None


def create_TCPSuperSpy(root):
    '''Starting point when module is imported by another program.'''
    global w, w_win
    if w:  # So we have only one instance of window.
        return
    w = Toplevel(root)
    w.title('TCPSuperSpy')
    w.geometry('800x600+339+204')
    w_win = TCPSuperSpy(w)
    init()
    return w_win


def destroy_TCPSuperSpy():
    global w
    w.destroy()
    w = None


def init():
    pass


def TODO():
        print ('TODO')


class TCPSuperSpy:
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
                command=root.quit,
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
        #self.Scrolledlistbox1 = ScrolledListBox(master)
        self.mlb.place(relx=0.0, rely=0.0, relheight=1.0, relwidth=0.5)
        #self.Scrolledlistbox1.place(relx=0.0, rely=0.0, relheight=1.0, relwidth=0.5)
        self.mlb.configure(selectbackground="#c4c4c4")
        #self.Scrolledlistbox1.configure(selectbackground="#c4c4c4")
        self.mlb.config(columns=('Local IP', 'Local Port', 'Remote IP', 'Remote Port', 'State'))


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
