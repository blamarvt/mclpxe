#!/bin/sh
# $1 is the filename
[ -z "$1" ] && exit 1
FILENAME='whatprovides3x.cgi?'$1
[ -f "$FILENAME" ] && rm "$FILENAME"
wget -q http://www.tinycorelinux.com/cgi-bin/"$FILENAME"
grep -v "^  <" "$FILENAME" > info.lst
rm "$FILENAME"
sed -i 's/\.list$//' info.lst
