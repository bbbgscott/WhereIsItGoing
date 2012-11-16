import sqlite3
import geo_helper as geo


#class sql_helper:
def create_tcp_table():  # Should only need to be run once
    conn = sqlite3.connect('../db/super_spy.db')
    c = conn.cursor()
    # Create table
    c.execute('''CREATE TABLE tcp (key INTEGER PRIMARY KEY,
                                    unix_time INTEGER,
                                    loc_ip VARCHAR(8),
                                    loc_port INTEGER,
                                    rem_ip VARCHAR(8),
                                    rem_port INTEGER,
                                    conn_stat BOOLEAN,
                                    exec_path VARCHAR(30))''')
    # Save changes (commit)
    conn.commit()

    # Close c
    c.close()


def insert_tcp_row(stamp, loc_ip, loc_port, rem_ip, rem_port, conn, ex_path):
    conn = sqlite3.connect('../db/super_spy.db')
    c = conn.cursor()
    local_ip = geo.convert_hex_ip(loc_ip)
    remote_ip = geo.convert_hex_ip(rem_ip)
    local_port = int(loc_port, 16)
    remote_port = int(rem_port, 16)
#    state = int(st, 16)
    row = stamp, local_ip, local_port, remote_ip, remote_port, conn, ex_path
    #c.execute('''INSERT INTO tcp (loc_ip, loc_port, rem_ip, rem_port, state)
    # VALUES (local_ip, local_port, remote_ip, remote_port, state)''')
    c.execute("INSERT INTO tcp(unix_time, loc_ip, loc_port, rem_ip, rem_port, conn_stat, exec_path) VALUES (?, ?, ?, ?, ?, ?, ?)", row)
    conn.commit()
    c.close()


def read_table():
    conn = sqlite3.connect('../db/super_spy.db')
    c = conn.cursor()
    c.execute("SELECT * FROM tcp")
    rows = c.fetchall()
    c.close()
    return rows


def drop_table():
    conn = sqlite3.connect('../db/super_spy.db')
    c = conn.cursor()
    c.execute("DROP TABLE tcp")
    c.close()


def get_distinct_ips():
    query = "SELECT DISTINCT FROM tcp"
    conn = sqlite3.connect('../db/super_spy.db')
    c = conn.cursor()
    c.execute(query)
    c.close()

'''
46: 010310AC:9C4C 030310AC:1770 01 00000150:00000000 01:00000019 00000000 1000 0 54165785 4 cd1e6040 25 4 27 3 -1
'''