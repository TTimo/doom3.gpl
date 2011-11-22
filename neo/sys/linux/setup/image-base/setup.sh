#! /bin/sh
#
# Product setup script
#
# Go to the proper setup directory (if not already there)
cd `dirname $0`

# defaults
FATAL_ERROR="Fatal error, no tech support email configured in this setup"
# try to get root prior to running setup?
# 0: no
# 1: prompt, but run anyway if fails
# 2: require, abort if root fails
GET_ROOT=0
XSU_ICON=""
# You may want to set USE_XHOST to 1 if you want an X11 application to
# be launched with root privileges right after installation
USE_XHOST=0
# this is the message for su call, printf
SU_MESSAGE="You need to run this installation as the super user.\nPlease enter the root password."

NULL=/dev/null
# See if we have the XPG4 utilities (Solaris)
if test -d /usr/xpg4/bin; then
	PATH=/usr/xpg4/bin:$PATH
fi

# Return the appropriate architecture string
DetectARCH()
{
	status=1
	case `uname -m` in
	    amd64 | x86_64)
		echo "amd64"
		status=0;;
	    i?86 | i86*)
		echo "x86"
		status=0;;
	    90*/*) 
		echo "hppa"
		status=0;;
	    *)
		case `uname -s` in
		    IRIX*)
			echo "mips"
			status=0;;
            AIX*)
            echo "ppc"
            status=0;;
		    *)
			arch=`uname -p 2> /dev/null || uname -m`
			if test "$arch" = powerpc; then
				echo "ppc"
			else
				echo $arch
			fi
			status=0;;
		esac
	esac
	return $status
}

# Return the appropriate version string
DetectLIBC()
{
	status=1
	if [ `uname -s` != Linux ]; then
		echo "glibc-2.1"
		return $status
	fi
	if [ -f `echo /lib/libc.so.6* | tail -n 1` ]; then
		# I'm not using glibc-specific binaries
		# this step even fails on amd64 gentoo, only has GLIBC_2.2 2.3 in it's strings
		echo "glibc-2.1"
		status=0
#		if [ fgrep GLIBC_2.1 /lib/libc.so.6* 2> $NULL >> $NULL ]; then
#			echo "glibc-2.1"
#			status=0
#		else    
#			echo "glibc-2.0"
#			status=0
#		fi        
	elif [ -f /lib/libc.so.5 ]; then
			echo "libc5"
			status=0
	else
			echo "unknown"
	fi
	return $status
}

DetectOS()
{
	os=`uname -s`
	if test "$os" = "OpenUNIX"; then
		echo SCO_SV
	else
		echo $os
	fi
	return 0
}

# Detect the environment
arch=`DetectARCH`
libc=`DetectLIBC`
os=`DetectOS`

args=""

# Import preferences from a secondary script
if [ -f setup.data/config.sh ]; then
    . setup.data/config.sh
elif [ -f SETUP.DAT/CONFIG.SH\;1 ]; then
	# HP-UX and other systems unable to get LFN correctly
	. SETUP.DAT/CONFIG.SH\;1
fi

# Add some standard paths for compatibility
PATH=$PATH:/usr/ucb

# call setup with -auth when ran through su/xsu
auth=0
if [ "$1" = "-auth" ]
then
  auth=1
  shift
fi

# Find the installation program
# try_run [-absolute] [-fatal] INSTALLER_NAME [PARAMETERS_PASSED]
#   -absolute option: if what you are trying to execute has an absolute path
#   NOTE: maybe try_run_absolute would be easier
#   -fatal option: if you want verbose messages in case
#      - the script could not be found
#      - it's execution would fail
#   INSTALLER_NAME: setup.gtk or setup
#   PARAMETERS_PASSED: additional arguments passed to the setup script
try_run()
{
    absolute=0
    if [ "$1" = "-absolute" ]; then
      absolute=1
      shift
    fi

    fatal=0
    # older bash < 2.* don't like == operator, using =
    if [ "$1" = "-fatal" ]; then
      # got fatal
      fatal=1
      shift
    fi

    setup=$1
    shift
    
    # First find the binary we want to run
    failed=0
    if [ "$absolute" -eq 0 ]
    then
      setup_bin="setup.data/bin/$os/$arch/$libc/$setup"
      # trying $setup_bin
      if [ ! -f "$setup_bin" ]; then
          setup_bin="setup.data/bin/$os/$arch/$setup"
        	# libc dependant version failed, trying again
          if [ ! -f "$setup_bin" ]; then
              failed=1
          fi
      fi
      if [ "$failed" -eq 1 ]; then
          if [ "$fatal" -eq 1 ]; then
              cat <<__EOF__
This installation doesn't support $libc on $os / $arch
(tried to run $setup)
$FATAL_ERROR
__EOF__
          fi
          return $failed
      fi

      # Try to run the binary ($setup_bin)
      # The executable is here but we can't execute it from CD
      # NOTE TTimo: this is dangerous, we also use $setup to store the name of the try_run
      setup="$HOME/.setup$$"
      rm -f "$setup"
      cp "$setup_bin" "$setup"    
      chmod 700 "$setup"
	  trap "rm -f $setup" 1 2 3 15
    fi
	# echo Running "$setup" "$@"
    if [ "$fatal" -eq 0 ]; then
        "$setup" "$@"
        failed="$?"
    else
        "$setup" "$@" 2>> $NULL
        failed="$?"
    fi
    if [ "$absolute" -eq 0 ]
    then
      # don't attempt removal when we are passed an absolute path
      # no, I don't want to imagine a faulty try_run as root on /bin/su
      rm -f "$setup"
    fi
    return "$failed"
}

if [ "$GET_ROOT" -eq 3 ]
then
	GOT_ROOT=`id -u`
	if [ "$GOT_ROOT" != "0" ]
	then
		printf "$SU_MESSAGE\n"
		echo "Press Enter to continue or Ctrl-C to abort"
		read
	fi
	GET_ROOT=0
fi

# if we have not been through the auth yet, and if we need to get root, then prompt
if [ "$auth" -eq 0 ] && [ "$GET_ROOT" -ne 0 ]
then
  GOT_ROOT=`id -u`
  if [ "$GOT_ROOT" != "0" ]
  then
	if [ "$USE_XHOST" -eq 1 ]; then
		xhost +127.0.0.1 2> $NULL > $NULL
	fi
    if [ "$GET_ROOT" -eq 1 ]
    then
      try_run xsu -e -a -u root -c "sh `pwd`/setup.sh -auth" $XSU_ICON -m "$XSU_MESSAGE"
    else
    try_run xsu -e -a -u root -c "sh `pwd`/setup.sh -auth" $XSU_ICON
    fi
    status="$?"
    # echo "got $status"
    # if try_run successfully executed xsu, it will return xsu's exit code
    # xsu returns 2 if ran and cancelled (i.e. the user 'doesn't want' to auth)
    # it will return 0 if the command was executed correctly
    # summing up, if we get 1, something failed
    if [ "$status" -eq 0 ]
    then
      # the auth command was properly executed
      exit 0
    elif [ "$status" -eq 1 ]
    then
      # xsu wasn't found, or failed to run
      # if xsu actually ran and the auth was cancelled, $status is > 1
      # try with su
      # su will return 1 if auth failed
      # we need to distinguish between su auth failed and su working, and still get setup.sh return code
      printf "$SU_MESSAGE\n"
      try_run -absolute /bin/su root -c "export DISPLAY=$DISPLAY;sh `pwd`/setup.sh -auth"
      status="$?"
	  if [ "$status" -eq 1 ] && [ "$GET_ROOT" -eq 1 ]
      then
        echo "Running setup as user"
      else
        exit $status
      fi
    elif [ "$status" -eq 3 ]
    then
      if [ "$GET_ROOT" -eq 1 ]
      then
        echo "Running setup as user"
      else
        # the auth failed or was canceled
        # we don't want to even start the setup if not root
        echo "Please run this installation as the super user"
        exit 1
      fi
    fi
    # continue running as is
  fi
fi

# run the setup program (setup.gtk first, then setup)
# except when trying setup.gtk first then console setup, always exit with 0
# if the setup was cancelled, it is not our problem
try_run setup.gtk $args $*
status="$?"
if [ "$status" -ne 0 ] && [ "$status" -ne 3 ]; then  # setup.gtk couldn't connect to X11 server - ignore
try_run setup $args $*
  status="$?"
  if [ "$auth" -eq 1 ] && [ "$status" -ne 0 ]
  then
    # distinguish between failed su and failed su'ed setup.sh
    exit 2
  fi
  exit $status
fi
