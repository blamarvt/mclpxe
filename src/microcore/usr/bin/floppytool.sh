#!/bin/sh
# floppytool.sh by J.P. Nimmo
#
if [ -z "$1" ]; then
	echo "Usage: floppytool.sh <function> <size> <path to save image>"
	echo "Possible functions are: format, makeimage, applyimage"
	exit 1
fi
#
if [ -z "$2" ]; then
	echo "Usage: floppytool.sh <function> <size> <path to save image>"
	echo "Possible functions are: format, makeimage, applyimage"
	exit 1
fi
#
if [ $1 == "format" ]; then
	sudo fdformat /dev/fd0H$2
	sudo mkdosfs -v /dev/fd0H$2
	sleep 5
fi
#
if [ $1 == "makeimage" ]; then
	echo "Creating Image"
	sudo dd if=/dev/fd0H$2 of=$3 bs=$2k count=1
	echo "Verifying Image"
	busybox cmp $3 /dev/fd0H$2
	echo "Successful!"
	sleep 5
fi
#
if [ $1 == "applyimage" ]; then
	echo "Applying Image to Floppy"
	sudo dd if=$3 of=/dev/fd0H$2 bs=$2k count=1
	echo "Verifying Floppy"
	busybox cmp /dev/fd0H$2 $3
	echo "Successful!"
	sleep 5
fi
#
