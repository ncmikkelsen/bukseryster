#!/usr/bin/env python
import feedparser
import urllib2
import time
import datetime
import socket
from IPy import IP

feedUrl="https://www.facebook.com/feeds/notifications.php?id=716307201&viewer=716307201&key=AWjvR9O-r-pnIqK1&format=rss20"
loopDelay=15

#get the feed and parse it:
def getNotifications(url):
	notifications=[]
	try:
		rawFeed=urllib2.urlopen(url)
		parsedFeed=feedparser.parse(rawFeed)

		#sort notifications into a set:
		for i in range(len(parsedFeed.entries)):
			notifications.append(parsedFeed.entries[i].published)
			#notifications.append(parsedFeed.entries[i].title)

	except Exception as e:
		print "hmm.. It looks like there was a network timeout: " + str(e)

	return notifications

#create timestamps from notification dates:
def getStamps(notifications):
	stamps=[]
	for i in range(len(notifications)):
									#rfc2822#section-3.3 format: "Tue, 26 May 2015 15:58:39 -0100"
		stamps.append(time.mktime(time.strptime(notifications[i], "%a, %d %b %Y %H:%M:%S -0100")))
	return stamps

#STARTUP:

print "%s Startup!" %time.ctime()

incomingPort = 31337
remotePort = 31337
# A UDP server listening for packets on port 31337:
UDPSock = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
listen_addr = ("",incomingPort)
UDPSock.bind(listen_addr)
UDPSock.settimeout(None) #timeout after 15 seconds, used as flow control in the while loop

oldTimeStamps=getStamps(getNotifications(feedUrl))
lastTime = int(time.time()) #Update time
newNotifications=0

while True:

	try:
		data,addr = UDPSock.recvfrom(1024)
		remoteIP=IP(addr[0]).strNormal() #convert address of packet origin to string
		print 'RX: "%s" @ %s from %s' % (data.rstrip('\n'), time.ctime(), remoteIP)

		#if int(time.time())-lastTime>loopDelay:
		newTimeStamps=getStamps(getNotifications(feedUrl))
		diff=[]
		if len(newTimeStamps)>0:
			diff = list(set(newTimeStamps) - set(oldTimeStamps)) #sets are subtractable, yay!
			oldTimeStamps=newTimeStamps
		newNotifications+=len(diff)
		#lastTime=time.time()
		print "New notifications: %s" % newNotifications

		MESSAGE="%s" % newNotifications
		UDPSock.sendto(MESSAGE, (remoteIP, remotePort))

		print 'TX: %s' % (MESSAGE)
		print "-------------"
		print
		newNotifications=0

	except:
		pass