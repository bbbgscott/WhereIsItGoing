import pygeoip

gi = pygeoip.GeoIP("db/GeoLiteCity.dat")
#print gi.record_by_name("google.com")
#print gi.record_by_addr("127.0.0.1")


def convert_ip(hex):

	x1 = int(hex[6:8], 16)
	x2 = int(hex[4:6], 16)
	x3 = int(hex[2:4], 16)
	x4 = int(hex[0:2], 16)
	iplist = str(x1), str(x2), str(x3), str(x4)
	fin = '.'.join(iplist)
	return fin

# print convert_ip("030310ac")
# should return 172.16.3.3
"""
print "You visited this IP address:"
convert_ip("9059bd5b")
print ", and they are located here:"
print gi.record_by_addr(convert_ip("9059bd5b"))
"""
