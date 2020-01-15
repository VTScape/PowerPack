#!bin/sh

pkill mserver
pkill multiserver

/home/hcchang/metertools/src/tools/mcontrol/mserver -m wattsup -s /dev/ttyUSB0 -p 6913 &
#sleep 10
#/home/hcchang/metertools/src/tools/mcontrol/multiserver &

#/etc/rc.d/init.d/httpd stop
#/etc/init.d/syslog stop
#rm /var/log/messages*
