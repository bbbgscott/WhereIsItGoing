import pygeoip

gi = pygeoip.GeoIP("db/GeoLiteCity.dat")
print gi.record_by_name("google.com")
