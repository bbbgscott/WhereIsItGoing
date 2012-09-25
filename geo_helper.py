import pygeoip

gi = pygeoip.GeoIP("db/GeoLiteCity.dat")
#print gi.record_by_name("google.com")
#print gi.record_by_addr("127.0.0.1")

ip = ""


def convert_ip(hex):
	global ip
	ip1 = hex[6:8]
	ip2 = hex[4:6]
	ip3 = hex[2:4]
	ip4 = hex[0:2]
	x1 = int(ip1, 16)
	x2 = int(ip2, 16)
	x3 = int(ip3, 16)
	x4 = int(ip4, 16)
	fin = str(x1) + '.' + str(x2) + '.' + str(x3) + '.' + str(x4)
	print fin
	ip = fin

convert_ip("030310ac")

print "You visited this IP address:"
convert_ip("9059bd5b")
print ", and they are located here:"
print gi.record_by_addr(ip)
