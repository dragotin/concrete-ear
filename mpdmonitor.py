#!/usr/bin/env python

# Concrete Ear is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Concrete Ear is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Concrete Ear.  If not, see <http://www.gnu.org/licenses/>.
#
# Copyright 2014- Klaas Freitag <concrete-ear@volle-kraft-voraus.de>
# 

import mpd
# from daemon import Daemon
import os
import time
import sys
import socket
import smbus

 
class MpdMonitor():

    def __init__(self):
	self.POLLING_INTERVAL = 5
	
	self.MPD_HOST = "volumio"
	self.MPD_PORT = 6600
	
	# Status defines
	self.PLAY = 1
	self.PAUSE = 2
	self.STOP = 3
	self.MAINTENANCE = 4
	self.ERROR = 5

	# Open the log file
	self.logFile = open("/tmp/mpdmonitor.log", "w", 0)

	# I2C bus
	self.bus = smbus.SMBus(1)
	self.address = 0x04
        time.sleep(0.2) # Give a bit of time to init

    def writeI2C( self, sendstring ):
        bd = []
        for value in sendstring:
            print "XX " + value + str(ord(value))
        bd.append(ord(value))

        self.bus.write_i2c_block_data(self.address, 0x00, bd)
        return -1

    # helper function to dump nested file structures to inspect.
    def dump(self, obj, nested_level=0, output=sys.stdout):
	spacing = '   '
	if type(obj) == dict:
	    print >> output, '%s{' % ((nested_level) * spacing)
	    for k, v in obj.items():
		if hasattr(v, '__iter__'):
		    print >> output, '%s%s:' % ((nested_level + 1) * spacing, k)
		    dump(v, nested_level + 1, output)
		else:
		    print >> output, '%s%s: %s' % ((nested_level + 1) * spacing, k, v)
	    print >> output, '%s}' % (nested_level * spacing)
	elif type(obj) == list:
	    print >> output, '%s[' % ((nested_level) * spacing)
	    for v in obj:
		if hasattr(v, '__iter__'):
		    dump(v, nested_level + 1, output)
		else:
		    print >> output, '%s%s' % ((nested_level + 1) * spacing, v)
	    print >> output, '%s]' % ((nested_level) * spacing)
	else:
	    print >> output, '%s%s' % (nested_level * spacing, obj)
        
    def log(self, line):
	ll = "{0}: {1}".format(time.strftime("%a, %d %b %Y %H:%M:%S"), line)
	print ll
	self.logFile.write(ll)
	
    def notify(self, state, song_info):
	""" Notify-function, add notification code here, ie. I2C"""
	# self.dump(song_info)
	self.log("Notify Status: {0}".format (state))
	self.writeI2C(state)
    
    def observe_mpd(self, client):
	"""This is the main function in the script. It observes mpd and notifies the user of any changes."""

	# Loop and detect mpd changes
	last_status = "Initial"
	last_song = "Initial"

	while True:
	     # Get status as a string, ie. 'play', 'pause' or 'stop'
	    current_status = client.status()['state']
	    # self.dump(client.status())
	    # There might be errors when getting song details if there is no song in the playlist
	    try:
		current_song = client.currentsong()['file']
		# Get song details
		artist = client.currentsong()['artist']
		album = client.currentsong()['album']
		title = client.currentsong()['title']
	    except KeyError:
		current_song, artist, album, title = ("", "", "", "")

	    self.notify(current_status, client.currentsong())
	    
	    # Save current status to compare with later
	    last_status = current_status
	    last_song = current_song
	    # Sleep for some time before checking status again
	    
	    client.idle()

    def run(self):
	"""Runs the notifier"""
	self.log( "mpdMonitor Running now.")
	
	# Initialise mpd client and wait till we have a connection
	while True:
	    try:
		client = mpd.MPDClient()
		client.connect(self.MPD_HOST, int(self.MPD_PORT))
		self.log("Connected to MPD")
		
		# Run the observer but watch for mpd crashes
		self.observe_mpd(client)
	    except KeyboardInterrupt:
		self.log("\nLater!")
		sys.exit()
	    except (socket.error, mpd.ConnectionError):
		self.log("Cannot connect to MPD")
		self.notify("error", [])
		time.sleep(self.POLLING_INTERVAL)

if __name__ == "__main__":
   monitor = MpdMonitor()
   monitor.run()
   
