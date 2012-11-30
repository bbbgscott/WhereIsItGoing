import time
import sqlite3
import geo_helper as geo


#class sql_helper:
def create_tcp_table():  # Should only need to be run once
    conn = sqlite3.connect('../db/super_spy.db')
    c = conn.cursor()
    # Create table
    c.execute("CREATE TABLE tcp (key INTEGER PRIMARY KEY, unix_time INTEGER, loc_ip TEXT, loc_port TEXT, rem_ip TEXT, rem_port TEXT, conn_stat TEXT, exec_path TEXT)")
    # Save changes (commit)
    conn.commit()

    # Close c
    c.close()


def insert_tcp_row(stamp, loc_ip, loc_port, rem_ip, rem_port, conn_stat, ex_path):
    conn = sqlite3.connect('../db/super_spy.db')
    c = conn.cursor()
    # Rewriting inputs
    local_ip = str(geo.convert_hex_ip(loc_ip))
    remote_ip = str(geo.convert_hex_ip(rem_ip))
    local_port = str(int(loc_port, 16))
    remote_port = str(int(rem_port, 16))
#    state = int(st, 16)
    row = stamp, local_ip, local_port, remote_ip, remote_port, conn_stat, ex_path
    #row = stamp, loc_ip, loc_port, rem_ip, rem_port, conn, ex_path
    #c.execute('''INSERT INTO tcp (loc_ip, loc_port, rem_ip, rem_port, state)
    # VALUES (local_ip, local_port, remote_ip, remote_port, state)''')
    c.execute('INSERT INTO tcp VALUES (null, ?, ?, ?, ?, ?, ?, ?)', row)
    #c.execute('INSERT INTO tcp VALUES (null, ?, ?, ?, ?, ?, ?, ?)', (stamp, loc_ip, loc_port, rem_ip, rem_port, conn_stat, ex_path))
    conn.commit()
    c.close()


def read_table():
    conn = sqlite3.connect('../db/super_spy.db')
    c = conn.cursor()
    c.execute("SELECT * FROM tcp")
    #c.execute("SELECT unix_time, loc_ip, loc_port, rem_ip, rem_port, conn_stat FROM tcp")
    rows = c.fetchall()
    c.close()
    return rows


def drop_table():
    conn = sqlite3.connect('../db/super_spy.db')
    c = conn.cursor()
    c.execute("DROP TABLE tcp")
    c.close()


def get_latest_id():
    conn = sqlite3.connect('../db/super_spy.db')
    c = conn.cursor()
    c.execute("SELECT * FROM tcp ORDER BY rowid DESC LIMIT 1")
    rowid = c.fetchall()
    c.close()
    return rowid


def get_distinct_ips():
    query = "SELECT DISTINCT rem_ip FROM tcp"
    conn = sqlite3.connect('../db/super_spy.db')
    c = conn.cursor()
    c.execute(query)
    rows = c.fetchall()
    c.close()
    return rows


def report_10_min():
    t = int(time.time())
    query = "SELECT * FROM tcp WHERE(" + t + " - unix_time < 600)"
    conn = sqlite3.connect('../db/super_spy.db')
    c = conn.cursor()
    c.execute(query)
    rows = c.fetchall()
    c.close()
    return rows


def report_hour():
    t = int(time.time()) - 3600
    conn = sqlite3.connect('../db/super_spy.db')
    c = conn.cursor()
    c.execute("SELECT * FROM tcp WHERE unix_time > " + t)
    rows = c.fetchall()
    c.close()
    return rows


def report_day():
    t = int(time.time())
    query = "SELECT * FROM tcp WHERE(" + t + " - unix_time < " + 86400 + ")"
    conn = sqlite3.connect('../db/super_spy.db')
    c = conn.cursor()
    c.execute(query)
    rows = c.fetchall()
    c.close()
    return rows


def general_report():
    endrow = []
    conn = sqlite3.connect('../db/super_spy.db')
    c = conn.cursor()
    c.execute("SELECT * FROM tcp")
    rows = c.fetchall()
    for row in rows:
        arow = geo.alter_row(row)
        endrow.append(arow)
    c.close()
    return endrow


def get_countries():
    endrow = []
    dict_ = {}
    conn = sqlite3.connect('../db/super_spy.db')
    c = conn.cursor()
    c.execute("SELECT * FROM tcp")
    rows = c.fetchall()
    c.close()
    for row in rows:
        arow = geo.alter_row(row)
        endrow.append(arow)

    for row in endrow:
        if row[2] not in dict_:
            dict_[row[2]] = 1
        else:
            dict_[row[2]] = dict_[row[2]] + 1
    return dict_


'''
46: 010310AC:9C4C 030310AC:1770 01 00000150:00000000 01:00000019 00000000 1000 0 54165785 4 cd1e6040 25 4 27 3 -1
'''
