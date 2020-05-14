#!/bin/sh

sleep .2

# kill dhcp client for eth0
sleep 1
if [ -f /var/run/udhcpc.eth0.pid ]; then
kill `cat /var/run/udhcpc.eth0.pid`
sleep 0.1
fi

# configure interface eth0
ifconfig eth0 192.168.1.254 netmask 255.255.255.0 broadcast 192.168.1.255 up

# Start the DHCP Server Process once the Interface is Ready with the IP Add
sleep .1
sudo udhcpd /etc/udhcpd.conf &

