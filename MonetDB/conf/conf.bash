#
# ! this file should be kept identical in !
# ! monet, sql, xml, acoi, template       !
#
# In the following. ${what} is one of monet, sql, xml, acoi. 
# It is automatically derived from the current directory name.
#
# This script is supposed to be "sourced" in the top-level directory of the
# checked-out ${what} source tree (referred to as BASE in the remainder). 
# While sourced, this script sets your (architecture dependent) environment
# as required to compile ${what}.
#
# For everything else but Monet, this script requires MONET_PREFIX to be
# set, or 'monet-config` to be in your PATH, in order to find your Monet
# installation.
#
# By default, compilation will take place in ${what}_BUILD=BASE/`uname` and ${what}
# will be installed in ${what}_PREFIX=${what}_BUILD. You can change either directory by
# setting the enviroment variables
#	${what}_BUILD
# and/or
#	${what}_PREFIX
# appropiately before "sourcing" this script.
#
# To select your desired compiler ("GNU" or "ntv" (native)), your desired
# binary type (32bit or 64bit), and whether binarys should be linked
# dynamically or statically, set the following environment variables before
# "sourcing" this script:
#
#	COMP="GNU"	or	COMP="ntv"
#	BITS="32"	or	BITS="64"
#	LINK="dynamic"	or	LINK="static"
#
# (If not or wrongly set, "GNU 32 dynamic" is used as default.)
#

if [ ! -f configure.ag  -a  ! -x configure ] ; then
	echo ''
	echo 'conf/conf.bash has to be "sourced" in the top-level directory of the checked-out ${what} source tree.'
	echo ''
	return 1
fi

base="`pwd`"
wh_t="`basename $base`"
what="`echo ${wh_t} | tr [:lower:] [:upper:]`"

if [ "${what}" != "MONET" ] ; then
	if [ ! "${MONET_PREFIX}" ] ; then
		MONET_PREFIX=`monet-config --prefix`
	fi
	if [ ! -x ${MONET_PREFIX}/bin/monet-config ] ; then
		echo ''
		echo 'Could not find Monet installation.'
		echo ''
		wh_t='' ; unset wh_t
		what='' ; unset what
		return 1
	fi
fi

eval WHAT_BUILD="\${${what}_BUILD}"
eval WHAT_PREFIX="\${${what}_PREFIX}"

binpath=""
libpath=""
modpath=""
conf_opts=""

os="`uname`"

# check for not or incorrectly set variables (${what}_BUILD, ${what}_PREFIX, COMP, BITS, LINK)

if [ ! "${WHAT_BUILD}" ] ; then
	echo ''
	echo ${what}'_BUILD not set to specify desired compilation directory.'
	echo 'Using '${what}'_BUILD="'${base}/${os}'" (default).'
	WHAT_BUILD="${base}/${os}"
fi
if [ ! "${WHAT_PREFIX}" ] ; then
	echo ''
	echo ${what}'_PREFIX not set to specify desired target directory.'
	echo 'Using '${what}'_PREFIX="'${WHAT_BUILD}'" (default).'
	WHAT_PREFIX="${WHAT_BUILD}"
fi

if [ "${COMP}" != "GNU"  -a  "${COMP}" != "ntv" ] ; then
	echo ''
	echo 'COMP not set to either "GNU" or "ntv" (native) to select the desired compiler.'
	echo 'Using COMP="GNU" (default).'
	COMP="GNU"
fi
if [ "${BITS}" != "32"   -a  "${BITS}" != "64"  ] ; then
	echo ''
	echo 'BITS not set to either "32" or "64" to select the desired binary type.'
	echo 'Using BITS="32" (default).'
	BITS="32"
fi
case "${LINK}" in
d*)	LINK="d";;
s*)	LINK="s";;
*)
	echo ''
	echo 'LINK not set to either "dynamic" or "static" to select the desired way of linking.'
	echo 'Using LINK="dynamic" (default).'
	LINK="d"
	;;
esac

# exclude "illegal" combinations

if [ "${os}" = "Linux" ] ; then
	if [ "${BITS}" = "64" ] ; then
		echo ''
		echo 'Linux doesn'\''t support 64 bit, yet; hence, using BITS="32".'
		BITS="32"
	fi
fi

# set default compilers, configure options & paths

if [ "${COMP}" = "GNU" ] ; then
	# standard GNU compilers are gcc/g++
	cc="gcc"
	cxx="g++"
fi
if [ "${COMP}" = "ntv" ] ; then
	# standard native compilers are cc/CC
	cc="cc"
	cxx="CC"
fi
if [ "${LINK}" = "d"   ] ; then
	# dynamic/shared linking
	conf_opts="${conf_opts} --enable-shared --disable-static"
  else
	# static linking
	conf_opts="${conf_opts} --disable-shared --enable-static"
fi
# "standard" local paths
binpath="/usr/local/bin:${binpath}"
libpath="/usr/local/lib:${libpath}"
# "our" /soft[64] path
soft32=/soft/local
soft64=/soft64/local
if [ "${BITS}" = "32" ] ; then
	softpath=${soft32}
  else
	softpath=${soft64}
fi

# (additional) system-specific settings

if [ "${os}" = "Linux" ] ; then
	if [ "${COMP}" = "ntv" ] ; then
		libpath="/soft/IntelC++-6.0-020312Z/lib:${libpath}"
		cc="icc"
		cxx="icc"
		if [ "${what}" = "MONET" ] ; then
			conf_opts="${conf_opts} --with-hwcounters=${softpath}"
		fi
	fi
fi

if [ "${os}" = "SunOS" ] ; then
	# "our" /soft[64] path on apps
	soft32="/var/tmp${soft32}"
	soft64="/var/tmp${soft64}"
	softpath="/var/tmp${softpath}"
	# "standard" SunOS paths
	binpath="/opt/SUNWspro/bin:/sw/SunOS/5.8/bin:/usr/java/bin:${binpath}"
	libpath="/sw/SunOS/5.8/lib:${libpath}"
	if [ "${BITS}" = "64" ] ; then
		# propper/extended LD_LIBRARY_PATH for 64bit on SunOS
		libpath="/usr/lib/sparcv9:/usr/ucblib/sparcv9:${libpath}"
		# GNU ar in /usr/local/bin doesn't support 64bit
		AR='/usr/ccs/bin/ar' ; export AR
		AR_FLAGS='-r -cu' ; export AR_FLAGS
		# libraries compiled with gcc may need the gcc libs, so
		# at them to the LD_LIBRARY_PATH 
		libpath="${soft32}/lib/sparcv9:${soft32}/lib:${libpath}"
	fi
	if [ "${COMP}${BITS}${LINK}" = "ntv32d" ] ; then
		# propper/extended LD_LIBRARY_PATH for native 32bit shared libs on SunOS
		libpath="/usr/ucblib:${libpath}"
	fi
	if [ "${COMP}${BITS}" = "GNU64" ] ; then
		# our gcc/g++ on apps is in ${soft32} (also for 64 bit)
		binpath="${soft32}/bin:${binpath}"
	fi
	if [ "${what}" = "SQL"  -a  "${COMP}" = "ntv" ] ; then
		# to find ltdl.h included by src/odbc/setup/drvcfg.c via odbcinstext.h
		cc="${cc} -I/usr/local/include"
	fi
fi

if [ "${os}" = "IRIX64" ] ; then
	# propper/extended paths on medusa
	binpath="/usr/local/egcs/bin:/usr/local/gnu/bin:/usr/java/bin:${binpath}"
	if [ "${BITS}" = "64" ] ; then
		# some tools are not in ${soft64} on medusa
		binpath="${soft32}/bin:${binpath}"
	fi
	if [ "${COMP}${BITS}" = "GNU64" ] ; then
		# our gcc/g++ on medusa is in ${soft32} (also for 64 bit)
		libpath="${soft32}/lib/mabi=64:${libpath}"
	fi
fi

## gathered from old scripts, but not used anymore/yet
#if [ "${os}" = "AIX" ] ; then
#	# rs6000.ddi.nl
#	# gcc/g++ only?
#	cc="${cc} -mthreads"
#	cxx="${cxx} -mthreads"
#fi
#if [ "${os}" = "CYGWIN32_NT" ] ; then
#	# yalite.ddi.nl
#	# gcc/g++ only!
#	cc="${cc} -mno-cygwin"   # ?
#	cxx="${cxx} -mno-cygwin" # ?
#	conf_opts="${conf_opts} --with-pthread=/${what}DS/PthreadsWin32"
#fi
#if [ "${os}" = "CYGWIN_NT-4.0" ] ; then
#	# VMware
#	# gcc/g++ only!
#	cc="${cc} -mno-cygwin"   # ?
#	cxx="${cxx} -mno-cygwin" # ?
#	conf_opts="${conf_opts} --with-pthread=/tmp"
#fi

if [ "${os}" != "Linux" ] ; then
	# on Linux, /soft/local is identical with /usr/local
	# prepend ${softpath} to ${binpath} & ${libpath}
	binpath="${softpath}/bin:${binpath}"
	libpath="${softpath}/lib:${libpath}"
	# "our" libs/tools in ${softpath}
	conf_opts="${conf_opts} --with-readline=${softpath}"
	case ${what} in
	MONET)
		conf_opts="${conf_opts} --with-z=${softpath}"
		conf_opts="${conf_opts} --with-bz2=${softpath}"
		;;
	ACOI)
		conf_opts="${conf_opts} --with-getopt=${softpath}"
		conf_opts="${conf_opts} --with-tcl=${softpath}"
		;;
	SQL)
		conf_opts="${conf_opts} --with-odbc=${softpath}"
		;;
	XML)
		conf_opts="${conf_opts} --with-expat=${softpath}"
		;;
	esac
fi

# CWI specific additional package settings
if [ -f conf/local.bash ]; then
	source conf/local.bash
fi

# tell configure about chosen compiler and bits
if [ "${cc}" != "gcc" ] ; then
	conf_opts="${conf_opts} --with-gcc='${cc}'"
fi
if [ "${cxx}" != "g++" ] ; then
	conf_opts="${conf_opts} --with-gxx='${cxx}'"
fi
if [ "${BITS}" = "64" ] ; then
	conf_opts="${conf_opts} --with-bits=${BITS}"
fi

if [ "${what}" != "MONET" ] ; then
	# tell configure where to find Monet
	conf_opts="${conf_opts} --with-monet=${MONET_PREFIX}"
fi

# prepend target bin-dir to PATH
binpath="${WHAT_PREFIX}/bin:${binpath}"

# the following is nolonger needed for Monet,
# but still needed for the rest:
if [ "${what}" != "MONET" ] ; then
	# set MONET_MOD_PATH and prepend it to LD_LIBRARY_PATH
	package="`egrep -h '^(AM_INIT_AUTOMAKE|PACKAGE).".*"' configure* | head -1 | perl -pe 's/^(AM_INIT_AUTOMAKE|PACKAGE)."(.*)".*$/$2/'`"
	modpath="${WHAT_PREFIX}/lib/${package}"
	libpath="${WHAT_PREFIX}/lib:${modpath}:${libpath}"
fi

# remove trailing ':'
binpath=`echo "${binpath}" | sed 's|:$||'`
libpath=`echo "${libpath}" | sed 's|:$||'`
modpath=`echo "${modpath}" | sed 's|:$||'`

# export new settings
echo ""
echo "Setting..."
CFLAGS="" ; export CFLAGS
echo " CFLAGS=${CFLAGS}"
CXXFLAGS="" ; export CXXFLAGS
echo " CXXFLAGS=${CXXFLAGS}"
if [ "${binpath}" ] ; then
	if [ "${PATH}" ] ; then
		# prepend new binpath to existing PATH, if PATH doesn't contain binpath, yet
		case ":${PATH}:" in
		*:${binpath}:*)
			;;
		*)
			PATH="${binpath}:${PATH}" ; export PATH
			;;
		esac
	  else
		# set PATH as binpath
		PATH="${binpath}" ; export PATH
	fi
	echo " PATH=${PATH}"
fi
if [ "${libpath}" ] ; then
	if [ "${LD_LIBRARY_PATH}" ] ; then
		# prepend new libpath to existing LD_LIBRARY_PATH, if LD_LIBRARY_PATH doesn't contain libpath, yet
		case ":${LD_LIBRARY_PATH}:" in
		*:${libpath}:*)
			;;
		*)
			LD_LIBRARY_PATH="${libpath}:${LD_LIBRARY_PATH}" ; export LD_LIBRARY_PATH
			;;
		esac
	  else
		# set LD_LIBRARY_PATH as libpath
		LD_LIBRARY_PATH="${libpath}" ; export LD_LIBRARY_PATH
	fi
	echo " LD_LIBRARY_PATH=${LD_LIBRARY_PATH}"
fi
if [ "${modpath}" ] ; then
	if [ "${MONET_MOD_PATH}" ] ; then
		# prepend new modpath to existing MONET_MOD_PATH, if MONET_MOD_PATH doesn't contain modpath, yet
		case ":${MONET_MOD_PATH}:" in
		*:${modpath}:*)
			;;
		*)
			MONET_MOD_PATH="${modpath}:${MONET_MOD_PATH}" ; export MONET_MOD_PATH
			;;
		esac
	  else
		# set MONET_MOD_PATH as modpath
		MONET_MOD_PATH="${modpath}" ; export MONET_MOD_PATH
	fi
	echo " MONET_MOD_PATH=${MONET_MOD_PATH}"
fi

# for convenience: store the complete configure-call in ${what}_CONFIGURE
WHAT_CONFIGURE="${base}/configure ${conf_opts} --prefix=${WHAT_PREFIX}"
echo " ${what}_CONFIGURE=${WHAT_CONFIGURE}"
eval "alias configure_${wh_t}='${WHAT_CONFIGURE}'"
eval "alias configure_${wh_t}"
eval "alias Mtest_${wh_t}='Mtest.py --TSTSRCBASE=${base} --TSTBLDBASE=${WHAT_BUILD} --TSTTRGBASE=${WHAT_PREFIX}'"
eval "alias Mtest_${wh_t}"

mkdir -p ${WHAT_BUILD}

echo ""
echo "To compile ${what}, just execute:"
echo -e "\t./bootstrap"
echo -e "\tcd ${WHAT_BUILD}"
echo -e "\tconfigure_${wh_t}"
echo -e "\tmake"
echo -e "\tmake install"
echo ""
echo "Then, to test ${what}, just execute:"
echo -e "\tcd ${base}"
echo -e "\tMtest_${wh_t} -r"
echo ""

eval "${what}_BUILD=\"$WHAT_BUILD\" ; export ${what}_BUILD"
eval "${what}_PREFIX=\"$WHAT_PREFIX\" ; export ${what}_PREFIX"
eval "${what}_CONFIGURE=\"$WHAT_CONFIGURE\" ; export ${what}_CONFIGURE"

wh_t='' ; unset wh_t
what='' ; unset what
WHAT_BUILD='' ; unset WHAT_BUILD
WHAT_PREFIX='' ; unset WHAT_PREFIX
WHAT_CONFIGURE='' ; unset WHAT_CONFIGURE

