#!/bin/sh
# opening URLs into your favorite web browser
# feel free to tweak this to your needs

if [ x`which firefox` != x ]
then
	firefox "$1"
elif [ x`which mozilla` != x ]
then
	mozilla "$1"
elif [ x`which opera` !=  x ]
then
	opera "$1"
else
	xterm -e lynx "$1"
fi

