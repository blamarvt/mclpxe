#!/bin/sh
# put other system startup commands here
ifconfig eth0 add 169.254.10.5/24
ifconfig eth0 up
/usr/sbin/udhcpd -f /usr/local/etc/udhcpd.conf &
/etc/init.d/services/tftpd start
