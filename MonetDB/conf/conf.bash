# The contents of this file are subject to the MonetDB Public
# License Version 1.0 (the "License"); you may not use this file
# except in compliance with the License. You may obtain a copy of
# the License at
# http://monetdb.cwi.nl/Legal/MonetDBLicense-1.0.html
# 
# Software distributed under the License is distributed on an "AS
# IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
# implied. See the License for the specific language governing
# rights and limitations under the License.
# 
# The Original Code is the Monet Database System.
# 
# The Initial Developer of the Original Code is CWI.
# Portions created by CWI are Copyright (C) 1997-2003 CWI.
# All Rights Reserved.
# 
# Contributor(s):
# 		Martin Kersten <Martin.Kersten@cwi.nl>
# 		Peter Boncz <Peter.Boncz@cwi.nl>
# 		Niels Nes <Niels.Nes@cwi.nl>
# 		Stefan Manegold  <Stefan.Manegold@cwi.nl>

#
# ! this file should be kept identical in                                    !
# ! MonetDB, sql, xml, acoi, gis, template, playpen, misq, times100, pruning !
#

# In the following, ${what} is one of monet, sql, xml, acoi. 
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
#	DEBUG="somevalue" 	or not set
#	WARNING="somevalue" 	or not set
#	PROFILE="somevalue"	or not set
#	OPTIMIZE="somevalue"	or not set
#	INSTRUMENT="somevalue"	or not set
#
# (If not or wrongly set, "GNU 32 dynamic" is used as default.)
#

if [ ! -f configure.ag  -a  ! -x configure ] ; then
	echo ''
	echo 'conf/conf.bash has to be "sourced" in the top-level directory of the checked-out ${what} source tree.'
	echo ''
	return 1
fi

# use the current directory's (base)name to guess, 
# which of monet, sql, xml, acoi, template, ... we are;
# allow the basename to be suffixed, e.g., by some 
# version number (`pwd`=="monet_4-3" => wh_t=="monet", 
# i.e., cut-off from the first non-letter till the end)
base="`pwd`"
wh_t="`basename $base | perl -pe 's|^([a-zA-Z]+)([^a-zA-Z].*)?$|$1|'`"
what="`echo ${wh_t} | tr [:lower:] [:upper:]`"

# Ugly hack: (Arjen)
if [ "${what}" = "MONETDB" ] ; then
  what="MONET"
fi

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

# set generic variables
eval WHAT_BUILD="\${${what}_BUILD}"
eval WHAT_PREFIX="\${${what}_PREFIX}"

binpath=""
libpath=""
modpath=""
mtest_modpath=""
conf_opts=""

os="`uname | sed 's|_NT-.*$||'`"
hw="`uname -m`"
host="`hostname`"

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
	if [ "${BITS}" = "64"  -a  "${hw}" != "ia64" ] ; then
		echo ''
		echo "${hw}"' is 32-bit, only; hence, using BITS="32".'
		BITS="32"
	  elif [ "${BITS}" = "32"  -a  "${hw}" = "ia64" ] ; then
		echo ''
		echo 'Currently, we do not support 32-bit on '"${hw}"'; hence, using BITS="64".'
		BITS="64"
	fi
fi

# set default compilers, configure options & paths

cc=''
cxx=''
if [ "${LINK}" = "d"   ] ; then
	# dynamic/shared linking
	conf_opts="${conf_opts} --enable-shared --disable-static"
  else
	# static linking
	conf_opts="${conf_opts} --disable-shared --enable-static"
fi
# "standard" local paths
if [ -d /usr/local ] ; then
	binpath="/usr/local/bin:${binpath}"
	libpath="/usr/local/lib:${libpath}"
fi
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
	if [ "${COMP}" = "ntv"  -a  -d "${softpath}" ] ; then
		# the Intel compiler doesn't find headers/libs in /usr/local without help
		case ${what} in
		MONET)
			conf_opts="${conf_opts} --with-hwcounters=${softpath}"
			conf_opts="${conf_opts} --with-pcl=${softpath}"
			;;
		PATHFINDER)
			conf_opts="${conf_opts} --with-gc=${softpath}"
			;;
		esac
	fi
	if [ "${hw}" = "ia64" ] ; then
		if [ "${host%.ins.cwi.nl}" = "titan" ] ; then
			# specific settings for our Itanium2 "titan" system
			binpath="/soft/python-2.3/bin:/soft/automake-1.5/bin:${binpath}"
			if [ "${COMP}" = "GNU" ] ; then
				binpath="/soft/gcc-3.3/bin:${binpath}"
				libpath="/soft/gcc-3.3/lib:${libpath}"
			fi
		  elif [ "${host%.sara.nl}" = "theo" ] ; then
			# specific settings for Sara's Itanium system
			binpath="/home/niels/soft/local/bin:${binpath}"
			libpath="/home/niels/soft/local/lib:${libpath}"
		fi
	fi
	if [ -x /net/lin_local/twig/twig ] ; then
		# twig in Konstanz
		binpath="/net/lin_local/twig:${binpath}"
	fi
	if [ -x /usr/lib/java/bin/javac  -a  -x /usr/lib/java/bin/jar ] ; then
		# java in Konstanz
		binpath="/usr/lib/java/bin:${binpath}"
	fi
fi

if [ "${os}" = "Darwin" ] ; then
	# "our" autoconf on sap & monet
	mypath=""
	for d in "/Users/manegold/soft/local/bin" "/Users/monet/soft/local/bin" "/usr/local/bin" "/usr/bin" "/sw/bin" ; do
		if [ -d ${d} ] ; then
			mypath="${mypath}${d}:"
		fi
	done
	binpath="${mypath}:${binpath}"
	unset mypath
fi

if [ "${os}" = "SunOS" ] ; then
	# "our" /soft[64] path on apps
	soft32="/var/tmp${soft32}"
	soft64="/var/tmp${soft64}"
	softpath="/var/tmp${softpath}"
	# "standard" SunOS paths
	binpath="/opt/SUNWspro/bin:/cwi/bin:/usr/java/bin:${binpath}"
	libpath="/cwi/lib:${libpath}"
	if [ "${BITS}" = "64" ] ; then
		# propper/extended LD_LIBRARY_PATH for 64bit on SunOS
		libpath="/usr/lib/sparcv9:/usr/ucblib/sparcv9:${libpath}"
		# GNU ar in /usr/local/bin doesn't support 64bit
		AR='/usr/ccs/bin/ar' ; export AR
		AR_FLAGS='-r -cu' ; export AR_FLAGS
		if [ -d "${soft32}" ] ; then
			# libraries compiled with gcc may need the gcc libs, so
			# at them to the LD_LIBRARY_PATH 
			libpath="${soft32}/lib/sparcv9:${soft32}/lib:${libpath}"
			# some tools are not in ${soft64} on apps
			binpath="${soft32}/bin:${binpath}"
		fi
	fi
	if [ "${COMP}${BITS}${LINK}" = "ntv32d" ] ; then
		# propper/extended LD_LIBRARY_PATH for native 32bit shared libs on SunOS
		libpath="/usr/ucblib:${libpath}"
	fi
	if [ "${what}" = "SQL"  -a  "${COMP}" = "ntv" ] ; then
		# to find ltdl.h included by src/odbc/setup/drvcfg.c via odbcinstext.h
		cc="cc -I/usr/local/include"
	fi
fi

if [ "${os}" = "IRIX64" ] ; then
	# propper/extended paths on medusa
	binpath="/usr/local/egcs/bin:/usr/local/gnu/bin:/usr/java/bin:${binpath}"
	if [ -d "${soft32}"  -a  "${BITS}" = "64" ] ; then
		# some tools are not in ${soft64} on medusa
		binpath="${soft32}/bin:${binpath}"
	fi
	if [ -d "${soft32}"  -a  "${COMP}${BITS}" = "GNU64" ] ; then
		# our gcc/g++ on medusa is in ${soft32} (also for 64 bit)
		libpath="${soft32}/lib/mabi=64:${libpath}"
	fi
fi

if [ "${os}" = "AIX" ] ; then
	# paths on beluga
	mypath=""
	for d in "/usr/local/bin" "/usr/local/tgcc-2.95.3/bin" "/usr/vac/bin" "/usr/vacpp/bin" "/opt/freeware/bin" "/usr/java131/jre/bin" "/usr/java131/bin" "/usr/ccs/bin" "/usr/ucb" "/usr/bin" "/usr/dt/bin" "/usr/bin/X11" "/usr/lpp/X11/bin" ; do
		if [ -d ${d} ] ; then
			mypath="${mypath}${d}:"
		fi
	done
	binpath="${mypath}:${binpath}"
	unset mypath
	# required to make mmap() work on AIX !
	export XPG_SUS_ENV=ON
fi

## gathered from old scripts, but not used anymore/yet
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

if [ "${os}" != "Linux"  -a  "${os}" != "CYGWIN"  -a  "${os}" != "Darwin" ] ; then
	# on Linux, CYGWIN, & Darwin, /soft/local is identical with /usr/local
	if [ -d "${softpath}" ] ; then
		# prepend ${softpath} to ${binpath} & ${libpath}
		binpath="${softpath}/bin:${binpath}"
		libpath="${softpath}/lib:${libpath}"
		# "our" libs/tools in ${softpath}
		conf_opts="${conf_opts} --with-readline=${softpath}"
		conf_opts="${conf_opts} --with-z=${softpath}"
		conf_opts="${conf_opts} --with-bz2=${softpath}"
		case ${what} in
		ACOI)
			conf_opts="${conf_opts} --with-getopt=${softpath}"
			conf_opts="${conf_opts} --with-tcl=${softpath}"
			;;
		PATHFINDER)
			conf_opts="${conf_opts} --with-gc=${softpath}"
			;;
		SQL)
			conf_opts="${conf_opts} --with-odbc=${softpath}"
			;;
		XML)
			conf_opts="${conf_opts} --with-expat=${softpath}"
			conf_opts="${conf_opts} --with-pcre=${softpath}"
			;;
		esac
	fi
elif [ "${os}" = "Linux"  -a  "${what}" = "PATHFINDER"  -a  -d /net/lin_local/libgc-devel ] ; then
	# libgc in Konstanz
	conf_opts="${conf_opts} --with-gc=/net/lin_local/libgc-devel"
fi

# CWI specific additional package settings
if [ -f conf/local.bash ]; then
	source conf/local.bash
fi

# tell configure about chosen compiler and bits
if [ "${cc}" ] ; then
	conf_opts="${conf_opts} --with-gcc='${cc}'"
elif [ "${COMP}" = "ntv" ] ; then
	conf_opts="${conf_opts} --with-gcc=no"
fi
if [ "${cxx}" ] ; then
	conf_opts="${conf_opts} --with-gxx='${cxx}'"
fi
if [ "${BITS}" = "64" ] ; then
	conf_opts="${conf_opts} --with-bits=${BITS}"
fi

if [ "${DEBUG}" ] ; then
	conf_opts="${conf_opts} --enable-debug"
fi

if [ "${WARNING}" ] ; then
	conf_opts="${conf_opts} --enable-warning"
fi

if [ "${PROFILE}" ] ; then
	conf_opts="${conf_opts} --enable-profile"
fi

if [ "${OPTIMIZE}" ] ; then
	conf_opts="${conf_opts} --enable-optimize"
fi

if [ "${INSTRUMENT}" ] ; then
	conf_opts="${conf_opts} --enable-instrument"
fi

if [ "${what}" != "MONET" ] ; then
	# tell configure where to find MonetDB
	conf_opts="${conf_opts} --with-monet=${MONET_PREFIX}"
fi

# prepend target bin-dir to PATH
binpath="${WHAT_PREFIX}/bin:${binpath}"

# the following is nolonger needed for Monet,
# but still needed for the rest:
if [ "${what}" != "MONET"  -a  "${WHAT_PREFIX}" != "${MONET_PREFIX}" ] ; then
	# set MONET_MOD_PATH and prepend it to LD_LIBRARY_PATH
	modpath="${WHAT_PREFIX}/lib/MonetDB"
	libpath="${WHAT_PREFIX}/lib:${modpath}:${libpath}"
	mtest_modpath="--monet_mod_path=`${MONET_PREFIX}/bin/monet-config --modpath`:${modpath}"
fi
if [ "${os}" = "IRIX64" ] ; then
	# IRIX64 requires this to find dependend modules
	if [ "${what}" = "MONET" ] ; then
		libpath="${WHAT_PREFIX}/lib/MonetDB:${libpath}"
	  else
		libpath="${MONET_PREFIX}/lib/MonetDB:${libpath}"
	fi
fi
if [ "${os}${COMP}${BITS}${what}" = "SunOSntv64MONET" ] ; then
	# native 64-bit version on SunOS needs this to find libmonet
	if [ "${what}" = "MONET" ] ; then
		libpath="${WHAT_PREFIX}/lib:${libpath}"
	  else
		libpath="${MONET_PREFIX}/lib:${libpath}"
	fi
fi

# remove trailing ':'
binpath=`echo "${binpath}" | sed 's|:$||'`
libpath=`echo "${libpath}" | sed 's|:$||'`
modpath=`echo "${modpath}" | sed 's|:$||'`

# export new settings
echo ""
echo "Setting..."
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
	if [ "${os}" = "Darwin" ] ; then
		if [ "${DYLD_LIBRARY_PATH}" ] ; then
			# prepend new libpath to existing DYLD_LIBRARY_PATH, if DYLD_LIBRARY_PATH doesn't contain libpath, yet
			case ":${DYLD_LIBRARY_PATH}:" in
			*:${libpath}:*)
				;;
			*)
				DYLD_LIBRARY_PATH="${libpath}:${DYLD_LIBRARY_PATH}" ; export DYLD_LIBRARY_PATH
				;;
			esac
		  else
			# set DYLD_LIBRARY_PATH as libpath
			DYLD_LIBRARY_PATH="${libpath}" ; export DYLD_LIBRARY_PATH
		fi
		echo " DYLD_LIBRARY_PATH=${DYLD_LIBRARY_PATH}"
	fi
	if [ "${os}" = "AIX" ] ; then
		if [ "${LIBPATH}" ] ; then
			# prepend new libpath to existing LIBPATH, if LIBPATH doesn't contain libpath, yet
			case ":${LIBPATH}:" in
			*:${libpath}:*)
				;;
			*)
				LIBPATH="${libpath}:${LIBPATH}" ; export LIBPATH
				;;
			esac
		  else
			# set LIBPATH as libpath
			LIBPATH="${libpath}" ; export LIBPATH
		fi
		echo " LIBPATH=${LIBPATH}"
	fi
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
eval "alias Mtest_${wh_t}='Mtest.py --TSTSRCBASE=${base} --TSTBLDBASE=${WHAT_BUILD} --TSTTRGBASE=${WHAT_PREFIX} ${mtest_modpath}'"
eval "alias Mtest_${wh_t}"
eval "alias Mapprove_${wh_t}='Mapprove.py --TSTSRCBASE=${base} --TSTBLDBASE=${WHAT_BUILD} --TSTTRGBASE=${WHAT_PREFIX}'"
eval "alias Mapprove_${wh_t}"

mkdir -p ${WHAT_BUILD}

cat <<EOF

To compile ${what}, just execute:
	./bootstrap
	cd ${WHAT_BUILD}
	configure_${wh_t}
	make
	make install

Then, to test ${what}, just execute:
	cd ${base}
	Mtest_${wh_t} -r

EOF

# set specific variables
eval "${what}_BUILD=\"$WHAT_BUILD\" ; export ${what}_BUILD"
eval "${what}_PREFIX=\"$WHAT_PREFIX\" ; export ${what}_PREFIX"
eval "${what}_CONFIGURE=\"$WHAT_CONFIGURE\" ; export ${what}_CONFIGURE"

# clean-up temporary variables
wh_t='' ; unset wh_t
what='' ; unset what
WHAT_BUILD='' ; unset WHAT_BUILD
WHAT_PREFIX='' ; unset WHAT_PREFIX
WHAT_CONFIGURE='' ; unset WHAT_CONFIGURE

