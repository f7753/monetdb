AC_DEFUN(AM_MONET,
[

dnl check for monet
have_monet=auto
MONETDIST="."
AC_ARG_WITH(monet,
[  --with-monet=DIR     monet is installed in DIR], have_monet="$withval")
if test "x$have_monet" != xno; then
  if test "x$have_monet" != xauto; then
    AC_CHECK_PROG(MONET_CONFIG,monet-config,$withval/bin/monet-config,,$withval/bin)
    MONETDIST=`$MONET_CONFIG --prefix`
    GDKLIBS=`$MONET_CONFIG --gdklibs`
    have_monet=yes
  else
    have_monet=yes
    AC_CHECK_PROG(MONET_CONFIG,monet-config,monet-config)
    MONETDIST=`$MONET_CONFIG --prefix`
    GDKLIBS=`$MONET_CONFIG --gdklibs`
  fi

  if test "x$have_monet" != xyes; then
    MONETDIST=""
    GDKLIBS=""
  fi
fi
AC_SUBST(MONETDIST)
AC_SUBST(GDKLIBS)
])

AC_DEFUN(AM_MONET_OPTIONS,
[
dnl --enable-debug
AC_ARG_ENABLE(debug,
[  --enable-debug          enable full debugging [default=off]],
  enable_debug=$enableval, enable_debug=no)
if test "x$enable_debug" = xyes; then
  if test "x$GCC" = xyes; then
    CFLAGS="$CFLAGS -O0"
  fi
  CFLAGS="$CFLAGS -g"
fi

dnl --enable-optimize
AC_ARG_ENABLE(optimize,
[  --enable-optimize       enable extra optimization [default=off]],
  enable_optim=$enableval, enable_optim=no)
if test "x$enable_optim" = xyes; then
  dnl Optimization flags
  if test "x$enable_debug" = xno; then
    if test "x$GCC" = xyes; then
      dnl -fomit-frame-pointer crashes memprof
      case "$host" in
      i*86-*-*)       CFLAGS="$CFLAGS -O6 -fomit-frame-pointer -finline-functions -malign-loops=4 -malign-jumps=4 -malign-functions=4 -ffast-math -fexpensive-optimizations -funroll-all-loops  -funroll-loops -frerun-cse-after-loop -frerun-loop-opt";;
      *-sun-solaris*) CFLAGS="$CFLAGS -O2 -fomit-frame-pointer -finline-functions";;
      *)              CFLAGS="$CFLAGS -O6 -fomit-frame-pointer -finline-functions";;
      esac
    else
      case "$host" in
      i*86-*-*)       CFLAGS="$CFLAGS -O3 -tpp6 -axiMK -unroll -wp_ipo -ipo_obj";;
      *irix6.5*)      CFLAGS="$CFLAGS -O3 -Ofast=IP27 -OPT:alias=restrict -IPA"
                      LDFLAGS="$LDFLAGS -IPA"
                      ;;
      *-sun-solaris*) CFLAGS="$CFLAGS -xO5"
                      ;;
      esac   
    fi

  fi
fi

dnl --enable-warning (only gcc)
AC_ARG_ENABLE(warning,
[  --enable-warning           enable extended compiler warnings [default=off]],
  enable_warning=$enableval, enable_warning=no)
if test "x$enable_warning" = xyes; then
  if test "x$GCC" = xyes; then
    CFLAGS="$CFLAGS -ansi -pedantic"
  fi
fi

dnl --enable-profile
need_profiling=no
AC_ARG_ENABLE(profile,
[  --enable-profile        enable profiling [default=off]],
  enable_prof=$enableval, enable_prof=no)
if test "x$enable_prof" = xyes; then
  CFLAGS="$CFLAGS -DPROFILE"
  need_profiling=yes
  if test "x$GCC" = xyes; then
    CFLAGS="$CFLAGS -pg"
  fi
fi
AM_CONDITIONAL(PROFILING,test "x$need_profiling" = xyes)

dnl --enable-instrument
need_instrument=no
AC_ARG_ENABLE(instrument,
[  --enable-instrument        enable instrument [default=off]],
  enable_instrument=$enableval, enable_instrument=no)
if test "x$enable_instrument" = xyes; then
  CFLAGS="$CFLAGS -DPROFILE"
  need_instrument=yes
  if test "x$GCC" = xyes; then
    CFLAGS="$CFLAGS -finstrument-functions -g"
  fi
fi


])

AC_DEFUN(AM_MONET_LIBS,
[
dnl libpthread
have_pthread=auto
PTHREAD_LIBS=""
PTHREAD_INCS=""
AC_ARG_WITH(pthread,
[  --with-pthread=DIR     pthread library is installed in DIR], 
	[have_pthread="$withval"])
if test "x$have_pthread" != xno; then
  if test "x$have_pthread" != xauto; then
    PTHREAD_LIBS="-L$withval/lib"
    PTHREAD_INCS="-I$withval/include"
  fi

  save_CPPFLAGS="$CPPFLAGS"
  CPPFLAGS="$CPPFLAGS $PTHREAD_INCS"
  AC_CHECK_HEADERS(pthread.h semaphore.h) 
  CPPFLAGS="$save_CPPFLAGS"

  save_LDFLAGS="$LDFLAGS"
  LDFLAGS="$LDFLAGS $PTHREAD_LIBS"
  AC_CHECK_LIB(pthread, sem_init, 
	[ PTHREAD_LIBS="$PTHREAD_LIBS -lpthread" 
          AC_DEFINE(HAVE_LIBPTHREAD) 
	  have_pthread=yes ] , 
	dnl sun
	[ AC_CHECK_LIB(pthread, sem_post, 
		[ PTHREAD_LIBS="$PTHREAD_LIBS -lpthread -lposix4" 
          	AC_DEFINE(HAVE_LIBPTHREAD) 
	  	have_pthread=yes ] , [ have_pthread=no], "-lposix4" )
	] )
  LDFLAGS="$save_LDFLAGS"

  if test "x$have_pthread" != xyes; then
    PTHREAD_LIBS=""
    PTHREAD_INCS=""
  fi
fi
AC_SUBST(PTHREAD_LIBS)
AC_SUBST(PTHREAD_INCS)

dnl libreadline
have_readline=auto
READLINE_LIBS=""
AC_ARG_WITH(readline,
[  --with-readline=DIR     readline library is installed in DIR], 
	[have_readline="$withval"])
if test "x$have_readline" != xno; then
  if test "x$have_readline" != xauto; then
    READLINE_LIBS="-L$withval/lib"
  fi

  save_LDFLAGS="$LDFLAGS"
  LDFLAGS="$LDFLAGS $READLINE_LIBS"
  AC_CHECK_LIB(readline, readline, 
	[ READLINE_LIBS="$READLINE_LIBS -lreadline -ltermcap" 
          AC_DEFINE(HAVE_LIBREADLINE) 
	  have_readline=yes ]
	, have_readline=no, "-ltermcap" )
  LDFLAGS="$save_LDFLAGS"

  if test "x$have_readline" != xyes; then
    READLINE_LIBS=""
  fi
fi
AC_SUBST(READLINE_LIBS)

DL_LIBS=""
AC_CHECK_LIB(dl, dlopen, [ DL_LIBS="-ldl" ] )
AC_SUBST(DL_LIBS)

MALLOC_LIBS=""
AC_CHECK_LIB(malloc, malloc, [ MALLOC_LIBS="-lmalloc" ] )
AC_SUBST(MALLOC_LIBS)

save_LDFLAGS="$LDFLAGS"
LDFLAGS="$LDFLAGS $MALLOC_LIBS"
AC_CHECK_FUNCS(mallopt mallinfo)
LDFLAGS="$save_LDFLAGS"


SOCKET_LIBS=""
AC_CHECK_LIB(socket, socket, [ SOCKET_LIBS="-lsocket -lnsl" ], [], "-lnsl" )
AC_SUBST(SOCKET_LIBS)

dnl check for z (de)compression library (default /usr and /usr/local)
have_z=auto
Z_CFLAGS=""
Z_LIBS=""
AC_ARG_WITH(z,
[  --with-z=DIR     z library is installed in DIR], have_z="$withval")
if test "x$have_z" != xno; then
  if test "x$have_z" != xauto; then
    Z_CFLAGS="-I$withval/include"
    Z_LIBS="-L$withval/lib"
  fi

  save_CPPFLAGS="$CPPFLAGS"
  CPPFLAGS="$CPPFLAGS $Z_CFLAGS"
  AC_CHECK_HEADER(zlib.h, have_z=yes, have_z=no)
  CPPFLAGS="$save_CPPFLAGS"

  if test "x$have_z" = xyes; then
  	save_LDFLAGS="$LDFLAGS"
  	LDFLAGS="$LDFLAGS $Z_LIBS"
  	AC_CHECK_LIB(z, gzopen, Z_LIBS="$Z_LIBS -lz"
        	AC_DEFINE(HAVE_LIBZ) have_z=yes, have_z=no)
  	LDFLAGS="$save_LDFLAGS"
  fi

  if test "x$have_z" != xyes; then
    Z_CFLAGS=""
    Z_LIBS=""
  fi
fi
AC_SUBST(Z_CFLAGS)
AC_SUBST(Z_LIBS)

dnl check for bz2 (de)compression library (default /usr and /usr/local)
have_bz=auto
BZ_CFLAGS=""
BZ_LIBS=""
AC_ARG_WITH(bz2,
[  --with-bz2=DIR     bz2 library is installed in DIR], have_bz="$withval")
if test "x$have_bz" != xno; then
  if test "x$have_bz" != xauto; then
    BZ_CFLAGS="-I$withval/include"
    BZ_LIBS="-L$withval/lib"
  fi

  save_CPPFLAGS="$CPPFLAGS"
  CPPFLAGS="$CPPFLAGS $BZ_CFLAGS"
  AC_CHECK_HEADER(bzlib.h, have_bz=yes, have_bz=no)
  CPPFLAGS="$save_CPPFLAGS"

  if test "x$have_bz" = xyes; then
  	save_LDFLAGS="$LDFLAGS"
  	LDFLAGS="$LDFLAGS $BZ_LIBS"
  	AC_CHECK_LIB(bz2, BZ2_bzopen, BZ_LIBS="$BZ_LIBS -lbz2"
        	AC_DEFINE(HAVE_LIBBZ2) have_bz=yes, have_bz=no)
  	LDFLAGS="$save_LDFLAGS"
  fi

  if test "x$have_bz" != xyes; then
    BZ_CFLAGS=""
    BZ_LIBS=""
  fi
fi
AC_SUBST(BZ_CFLAGS)
AC_SUBST(BZ_LIBS)

dnl libgetopt
have_getopt=auto
GETOPT_LIBS=""
GETOPT_INCS=""
AC_ARG_WITH(getopt,
[  --with-getopt=DIR     getopt library is installed in DIR], 
	[have_getopt="$withval"])
if test "x$have_getopt" != xno; then
  if test "x$have_getopt" != xauto; then
    GETOPT_LIBS="-L$withval/lib"
    GETOPT_INCS="-I$withval/include"
  fi

  save_CPPFLAGS="$CPPFLAGS"
  CPPFLAGS="$CPPFLAGS $GETOPT_INCS"
  AC_CHECK_HEADERS(getopt.h) 
  CPPFLAGS="$save_CPPFLAGS"

  save_LDFLAGS="$LDFLAGS"
  LDFLAGS="$LDFLAGS $GETOPT_LIBS"
  AC_CHECK_LIB(c, getopt_long, 
	[ GETOPT_LIBS="$GETOPT_LIBS" 
          AC_DEFINE(HAVE_LIBGETOPT) 
	  have_getopt=yes ] , 
	[ AC_CHECK_LIB(getopt, getopt_long_only, 
		[ GETOPT_LIBS="$GETOPT_LIBS -lgetopt" 
          	AC_DEFINE(HAVE_LIBGETOPT) 
	  	have_getopt=yes ] , [ have_getopt=no] )
	] )
  LDFLAGS="$save_LDFLAGS"

  if test "x$have_getopt" != xyes; then
    GETOPT_LIBS=""
    GETOPT_INCS=""
  fi
fi
AC_SUBST(GETOPT_LIBS)
AC_SUBST(GETOPT_INCS)
AM_CONDITIONAL(HAVE_GETOPT,test x$have_getopt = xyes)


])
