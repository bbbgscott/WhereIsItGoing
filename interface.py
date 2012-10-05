from Tkinter import *
import geo_helper
import sqlite_helper


class App:
	def __init__(self, master):
		frame = Frame(master)
		frame.pack()

		self.button = Button(frame, text="Quit", fg="red", command=frame.quit)
		self.button.pack(side=LEFT)

		self.hello = Button(frame, text="Hello", command=self.say_hi)
		self.hello.pack(side=LEFT)

		menubar = Menu(root)
		filemenu = Menu(menubar, tearoff=0)
		filemenu.add_command(label="Open", command=self.say_hi)
		filemenu.add_command(label="Save", command=self.say_hi)
		filemenu.add_separator()
		filemenu.add_command(label="Exit", command=root.quit)
		menubar.add_cascade(label="File", menu=filemenu)
		root.config(menu=menubar)

	def say_hi(self):
		print "hi there, everyone."


root = Tk()

app = App(root)

root.mainloop()
