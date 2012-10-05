import sqlite3


#class sql_helper:
def create_tcp_table():  # Should only need to be run once

    conn = sqlite3.connect('db/super_spy.db')
    c = conn.cursor()
    # Create table
    c.execute('''CREATE TABLE tcp (key INTEGER PRIMARY KEY,
    							   loc_ip VARCHAR(8),
    							   loc_port VARCHAR(8),
    							   rem_ip VARCHAR(8),
    							   rem_port VARCHAR(8),
    							   state VARCHAR(2))''')
    # Save changes (commit)
    conn.commit()

    # Close c
    c.close()


def insert_tcp_row(loc_ip, loc_port, rem_ip, rem_port, state):

	conn = sqlite3.connect('db/super_spy.db')
	c = conn.cursor()
	c.execute('''INSERT INTO tcp (loc_ip, loc_port, rem_ip, rem_port, state)
		VALUES (loc_ip, loc_port, rem_ip, rem_port, state)''')
	conn.commit()
	c.close()


def read_table():

    conn = sqlite3.connect('db/super_spy.db')
    c = conn.cursor()
    rows = c.execute("SELECT * FROM tcp")
    c.close()
    return rows


def drop_table():

    conn = sqlite3.connect('db/super_spy.db')
    c = conn.cursor()
    c.execute("DROP TABLE tcp")

'''
46: 010310AC:9C4C 030310AC:1770 01 00000150:00000000 01:00000019 00000000 1000 0 54165785 4 cd1e6040 25 4 27 3 -1
'''
