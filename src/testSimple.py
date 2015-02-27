import httplib
import sys
from random import Random

def random_str(randomlength=8):
    str = ''
    chars = 'AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz0123456789'
    length = len(chars) - 1
    random = Random()
    for i in range(randomlength):
        str+=chars[random.randint(0, length)]
    return str

if (len(sys.argv) != 2):
	print "Please input the argument as <port_number> <filename>"
	sys.exit(1)

# get the argument from command input
port_number = sys.argv[1]
# path = argv[1]

# test filename 
filename = ["/index.html", "/style.css", "/images/server_attention_span.png", "/images/1.png"]

# test a long long uri input
filename.append(random_str(1025))

if port_number.isdigit():
	port_number = int(port_number)
	if port_number < 65535 and port_number > 1024:
		
		conn = httplib.HTTPConnection("128.237.209.204", port_number, timeout = 30)

		print "-------------------------------"
		print "sending get http request"
		for x in filename:
			print "GET ", x
			conn.request("GET", x)
			r1 = conn.getresponse()
			if r1.version == 10:
				if r1.status >= 200 and r1.status < 400:
					print "HTTP/1.0", r1.status, r1.reason
					print "Server: ", r1.getheaders()[1][1]
					print "Content-Type:", r1.getheaders()[0][1]
				else:
					print "HTTP/1.0", r1.status, r1.reason[:9]
					print r1.reason[9:]
			elif r1.version == 11:
				if r1.status >= 200 and r1.status < 400:
					print "HTTP/1.0", r1.status, r1.reason
					print "Server: ", r1.getheaders()[1][1]
					print "Content-Type:", r1.getheaders()[0][1]
				else:
					print "HTTP/1.0", r1.status, r1.reason[:9]
					print r1.reason[9:]
			else:
				print "HTTP version error"
				sys.exit(1)
			print

		print "----------------------------------"
		print "sending head http request"
		for x in xrange(0, len(filename)):
			print "HEAD ", filename[x]
			conn.request("HEAD", filename[x])
			r1 = conn.getresponse()
			if r1.version == 10:
				if r1.status >= 200 and r1.status < 400:
					print "HTTP/1.0", r1.status, r1.reason
					print "Server: ", r1.getheaders()[1][1]
					print "Content-Type:", r1.getheaders()[0][1]
				else:
					print "HTTP/1.0", r1.status, r1.reason[:9]
					print r1.reason[9:]
			elif r1.version == 11:
				if r1.status >= 200 and r1.status < 400:
					print "HTTP/1.0", r1.status, r1.reason
					print "Server: ", r1.getheaders()[1][1]
					print "Content-Type:", r1.getheaders()[0][1]
				else:
					print "HTTP/1.0", r1.status, r1.reason[:9]
					print r1.reason[9:]
			else:
				print "HTTP version error"
				sys.exit(1)

		print "----------------------------------"
		print "sending post http request"
		for x in xrange(0, len(filename)):
			print "POST ", filename[x]
			conn.request("POST", filename[x])
			r1 = conn.getresponse()
			if r1.version == 10:
				if r1.status >= 200 and r1.status < 400:
					print "HTTP/1.0", r1.status, r1.reason
					print "Server: ", r1.getheaders()[1][1]
					print "Content-Type:", r1.getheaders()[0][1]
				else:
					print "HTTP/1.0", r1.status, r1.reason[:9]
					print r1.reason[9:]
			elif r1.version == 11:
				if r1.status >= 200 and r1.status < 400:
					print "HTTP/1.0", r1.status, r1.reason
					print "Server: ", r1.getheaders()[1][1]
					print "Content-Type:", r1.getheaders()[0][1]
				else:
					print "HTTP/1.0", r1.status, r1.reason[:9]
					print r1.reason[9:]
			else:
				print "HTTP version error"
				sys.exit(1)


		conn.close()
		sys.exit(1)
else:
	print port_number
	print "Please input the correct port_number in range 1024 to 65535"
	sys.exit(1)