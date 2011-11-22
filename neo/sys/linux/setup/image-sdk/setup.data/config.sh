#
# Use this script to customize the installer bootstrap script
#

# override some defaults

# try to get root prior to running setup?
# 0: no
# 1: prompt, but run anyway if fails
# 2: require, abort if root fails
GET_ROOT=0

FATAL_ERROR="Error running installer. See http://zerowing.idsoftware.com/linux/ for troubleshooting"

#XSU_ICON="-i icon.xpm"

#SU_MESSAGE="It is recommended to run this installation as root.\nPlease enter the root password, or hit return to continue as user"
