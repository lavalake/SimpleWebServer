# -*- coding: utf-8 -*-
import threading
import time
import httplib
import sys

if (len(sys.argv) != 3):
    print "Please input the argument as <port_number> <server_address>"
    sys.exit(1)
# host address
HOST = sys.argv[2]
# port number
PORT = sys.argv[1]
# test URI
URI = "/index.html"
# total connection number
TOTAL = 0
# success connection number
SUCC = 0
# failed conncetion number
FAIL = 0
# exception connection number
EXCEPT = 0

class RequestThread(threading.Thread):

    def __init__(self, thread_name):
        threading.Thread.__init__(self)
        self.test_count = 0

    # run thread
    def run(self):
        self.test_performace()

    def test_performace(self):
            global TOTAL
            global SUCC
            global FAIL
            global EXCEPT

            try:
                conn = httplib.HTTPConnection(HOST, PORT, False)
                conn.request('GET', URI)
                res = conn.getresponse()
                # print 'version:', res.version
                # print 'reason:', res.reason
                # print 'status:', res.status
                # print 'headers:', res.getheaders()

                if res.status == 200:
                    TOTAL += 1
                    SUCC += 1
                else:
                    TOTAL += 1
                    FAIL += 1

            except Exception, e:
                print e
                TOTAL += 1
                EXCEPT += 1
            conn.close()

print '===========task start==========='

# set up 300 threads
thread_count = 300

i = 0
while i <= thread_count:
    t = RequestThread("thread" + str(i))
    t.start()
    i += 1

# end
while TOTAL < thread_count:
    time.sleep(1)

print '===========task end==========='
# print out the result
print "total:%d, succ:%d, fail:%d, except:%d" %(TOTAL, SUCC, FAIL, EXCEPT)
