#
# Use this script to customize the installer bootstrap script
#

# override some defaults

# try to get root prior to running setup?
# 0: no
# 1: prompt, but run anyway if fails
# 2: require, abort if root fails
# 3: print SU_MESSAGE if not root, don't attempt any privilege upgrade
GET_ROOT=1

FATAL_ERROR="Error running installer. See http://zerowing.idsoftware.com/linux/doom/ for troubleshooting"

#XSU_ICON="-i icon.xpm"

SU_MESSAGE="The recommended install location (/usr/local/games) requires root permissions.\nPlease enter the root password or hit enter to continue install as current user."
XSU_MESSAGE="The recommended install location (/usr/local/games) requires root permissions.^Please enter the root password or hit cancel to continue install as current user."

rm -f /usr/local/games/tmp.$$ > /dev/null 2>&1
touch /usr/local/games/tmp.$$ > /dev/null 2>&1
status="$?"
if [ "$status" -eq 0 ]
then
  rm -f /usr/local/games/tmp.$$
  GET_ROOT=0
else
  GET_ROOT=1
fi
