#!/bin/sh
# $1 is the search word(s)
[ -z "$1" ] && exit 1
FILENAME='tcz3x.cgi?'$1
[ -f "$FILENAME" ] && rm "$FILENAME"
wget -q http://www.tinycorelinux.com/cgi-bin/"$FILENAME"
grep -v "^  <" "$FILENAME" > info.lst
rm "$FILENAME"
sed -i 's/\.info$//' info.lst
