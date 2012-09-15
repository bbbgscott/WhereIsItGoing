#! /usr/bin/python

import sqlite3


def create_tcp_table():

    conn = sqlite3.connect('db/super_spy.db')

    c = conn.cursor()

    # Create table
    c.execute('''CREATE TABLE tcp
                 (date text, trans text, symbol text, qty real, price real)''')

    # Insert row of data
    c.execute("INSERT INTO tcp VALUES ('2006-01-05','BUY','RHAT',100,35.14)")

    # Save changes (commit)
    conn.commit()

    # Close cursor
    c.close()


def insert_tcp_row(arg1, arg2, arg3, arg4, arg5):

	conn = sqlite3.connect('db/super_spy.db')
	c = conn.cursor()
	c.execute("INSERT INTO ")


'''
46: 010310AC:9C4C 030310AC:1770 01 00000150:00000000 01:00000019 00000000 1000 0 54165785 4 cd1e6040 25 4 27 3 -1
'''
