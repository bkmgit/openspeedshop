# ===========================================================================
#          http://www.nongnu.org/autoconf-archive/ax_boost_base.html
# ===========================================================================
#
# SYNOPSIS
#
#   AX_BOOST_BASE([MINIMUM-VERSION], [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
#
# DESCRIPTION
#
#   Test for the Boost C++ libraries of a particular version (or newer)
#
#   If no path to the installed boost library is given the macro searchs
#   under /usr, /usr/local, /opt and /opt/local and evaluates the
#   $BOOST_ROOT environment variable. Further documentation is available at
#   <http://randspringer.de/boost/index.html>.
#
#   This macro calls:
#
#     AC_SUBST(BOOST_CPPFLAGS) / AC_SUBST(BOOST_LDFLAGS)
#
#   And sets:
#
#     HAVE_BOOST
#
# LICENSE
#
#   Copyright (c) 2008 Thomas Porschberg <thomas@randspringer.de>
#   Copyright (c) 2009 Peter Adolphs
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved. This file is offered as-is, without any
#   warranty.

#serial 11

AC_DEFUN([AX_BOOST_BASE],
[
AC_ARG_WITH([boost],
  [AC_HELP_STRING([--with-boost@<:@=ARG@:>@],
    [use Boost library from a standard location (ARG=yes),
     from the specified location (ARG=<path>),
     or disable it (ARG=no)
     @<:@ARG=yes@:>@ ])],
    [
    if test "$withval" = "no"; then
        want_boost="no"
    elif test "$withval" = "yes"; then
        want_boost="yes"
        ac_boost_path=""
    else
        want_boost="yes"
        ac_boost_path="$withval"
    fi
    ],
    [want_boost="yes"])


AC_ARG_WITH([boost-libdir],
        AS_HELP_STRING([--with-boost-libdir=LIB_DIR],
        [Force given directory for boost libraries. Note that this will overwrite library path detection, so use this parameter only if default library detection fails and you know exactly where your boost libraries are located.]),
        [
        if test -d $withval
        then
                ac_boost_lib_path="$withval"
        else
                AC_MSG_ERROR(--with-boost-libdir expected directory name)
        fi
        ],
        [ac_boost_lib_path=""]
)

if test "x$want_boost" = "xyes"; then
    boost_lib_version_req=ifelse([$1], ,1.20.0,$1)
    boost_lib_version_req_shorten=`expr $boost_lib_version_req : '\([[0-9]]*\.[[0-9]]*\)'`
    boost_lib_version_req_major=`expr $boost_lib_version_req : '\([[0-9]]*\)'`
    boost_lib_version_req_minor=`expr $boost_lib_version_req : '[[0-9]]*\.\([[0-9]]*\)'`
    boost_lib_version_req_sub_minor=`expr $boost_lib_version_req : '[[0-9]]*\.[[0-9]]*\.\([[0-9]]*\)'`
    if test "x$boost_lib_version_req_sub_minor" = "x" ; then
        boost_lib_version_req_sub_minor="0"
        fi
    WANT_BOOST_VERSION=`expr $boost_lib_version_req_major \* 100000 \+  $boost_lib_version_req_minor \* 100 \+ $boost_lib_version_req_sub_minor`
    AC_MSG_CHECKING(for boostlib >= $boost_lib_version_req)
    succeeded=no

    libsubdir="lib"
    if test "$(uname -m)" = "x86_64"; then
        libsubdir="lib64"
    fi

    dnl first we check the system location for boost libraries
    dnl this location ist chosen if boost libraries are installed with the --layout=system option
    dnl or if you install boost with RPM
    if test "$ac_boost_path" != ""; then
	BOOST_DETECTED_PATH="$ac_boost_path"
        BOOST_LDFLAGS="-L$ac_boost_path/$libsubdir"
        BOOST_CPPFLAGS="-I$ac_boost_path/include"
    elif test "$cross_compiling" != yes; then
        for ac_boost_path_tmp in /usr /usr/local /opt /opt/local ; do
            if test -d "$ac_boost_path_tmp/include/boost" && test -r "$ac_boost_path_tmp/include/boost"; then
		BOOST_DETECTED_PATH="$ac_boost_path_tmp"
                BOOST_LDFLAGS="-L$ac_boost_path_tmp/$libsubdir"
                BOOST_CPPFLAGS="-I$ac_boost_path_tmp/include"
                break;
            fi
        done
    fi

    dnl overwrite ld flags if we have required special directory with
    dnl --with-boost-libdir parameter
    if test "$ac_boost_lib_path" != ""; then
       BOOST_LDFLAGS="-L$ac_boost_lib_path"
    fi

    CPPFLAGS_SAVED="$CPPFLAGS"
    CPPFLAGS="$CPPFLAGS $BOOST_CPPFLAGS"
    export CPPFLAGS

    LDFLAGS_SAVED="$LDFLAGS"
    LDFLAGS="$LDFLAGS $BOOST_LDFLAGS"
    export LDFLAGS

    AC_REQUIRE([AC_PROG_CXX])
    AC_LANG_PUSH(C++)
        AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
    @%:@include <boost/version.hpp>
    ]], [[
    #if BOOST_VERSION >= $WANT_BOOST_VERSION
    // Everything is okay
    #else
    #  error Boost version is too old
    #endif
    ]])],[
        AC_MSG_RESULT(yes)
    succeeded=yes
    found_system=yes
        ],[
        ])
    AC_LANG_POP([C++])



    dnl if we found no boost with system layout we search for boost libraries
    dnl built and installed without the --layout=system option or for a staged(not installed) version
    if test "x$succeeded" != "xyes"; then
        _version=0
        if test "$ac_boost_path" != ""; then
            if test -d "$ac_boost_path" && test -r "$ac_boost_path"; then
                for i in `ls -d $ac_boost_path/include/boost-* 2>/dev/null`; do
                    _version_tmp=`echo $i | sed "s#$ac_boost_path##" | sed 's/\/include\/boost-//' | sed 's/_/./'`
                    V_CHECK=`expr $_version_tmp \> $_version`
                    if test "$V_CHECK" = "1" ; then
                        _version=$_version_tmp
                    fi
                    VERSION_UNDERSCORE=`echo $_version | sed 's/\./_/'`
                    BOOST_CPPFLAGS="-I$ac_boost_path/include/boost-$VERSION_UNDERSCORE"
                done
            fi
        else
            if test "$cross_compiling" != yes; then
                for ac_boost_path in /usr /usr/local /opt /opt/local ; do
                    if test -d "$ac_boost_path" && test -r "$ac_boost_path"; then
                        for i in `ls -d $ac_boost_path/include/boost-* 2>/dev/null`; do
                            _version_tmp=`echo $i | sed "s#$ac_boost_path##" | sed 's/\/include\/boost-//' | sed 's/_/./'`
                            V_CHECK=`expr $_version_tmp \> $_version`
                            if test "$V_CHECK" = "1" ; then
                                _version=$_version_tmp
                                best_path=$ac_boost_path
                            fi
                        done
                    fi
                done

                VERSION_UNDERSCORE=`echo $_version | sed 's/\./_/'`
                BOOST_CPPFLAGS="-I$best_path/include/boost-$VERSION_UNDERSCORE"
                if test "$ac_boost_lib_path" = ""
                then
                   BOOST_LDFLAGS="-L$best_path/$libsubdir"
                fi
            fi

            if test "x$BOOST_ROOT" != "x"; then
                if test -d "$BOOST_ROOT" && test -r "$BOOST_ROOT" && test -d "$BOOST_ROOT/stage/$libsubdir" && test -r "$BOOST_ROOT/stage/$libsubdir"; then
                    version_dir=`expr //$BOOST_ROOT : '.*/\(.*\)'`
                    stage_version=`echo $version_dir | sed 's/boost_//' | sed 's/_/./g'`
                        stage_version_shorten=`expr $stage_version : '\([[0-9]]*\.[[0-9]]*\)'`
                    V_CHECK=`expr $stage_version_shorten \>\= $_version`
                    if test "$V_CHECK" = "1" -a "$ac_boost_lib_path" = "" ; then
                        AC_MSG_NOTICE(We will use a staged boost library from $BOOST_ROOT)
                        BOOST_CPPFLAGS="-I$BOOST_ROOT"
                        BOOST_LDFLAGS="-L$BOOST_ROOT/stage/$libsubdir"
                    fi
                fi
            fi
        fi

        CPPFLAGS="$CPPFLAGS $BOOST_CPPFLAGS"
        export CPPFLAGS
        LDFLAGS="$LDFLAGS $BOOST_LDFLAGS"
        export LDFLAGS

        AC_LANG_PUSH(C++)
            AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
        @%:@include <boost/version.hpp>
        ]], [[
        #if BOOST_VERSION >= $WANT_BOOST_VERSION
        // Everything is okay
        #else
        #  error Boost version is too old
        #endif
        ]])],[
            AC_MSG_RESULT(yes)
        succeeded=yes
        found_system=yes
            ],[
            ])
        AC_LANG_POP([C++])
    fi

    if test "x$BOOST_DETECTED_PATH" == "x/usr"; then
        BOOST_LDFLAGS=""
        BOOST_CPPFLAGS=""
    fi

    if test "$succeeded" != "yes" ; then
        if test "$_version" = "0" ; then
            AC_MSG_NOTICE([[We could not detect the boost libraries (version $boost_lib_version_req_shorten or higher). If you have a staged boost library (still not installed) please specify \$BOOST_ROOT in your environment and do not give a PATH to --with-boost option.  If you are sure you have boost installed, then check your version number looking in <boost/version.hpp>. See http://randspringer.de/boost for more documentation.]])
        else
            AC_MSG_NOTICE([Your boost libraries seems to old (version $_version).])
        fi
        # execute ACTION-IF-NOT-FOUND (if present):
        ifelse([$3], , :, [$3])
    else
        AC_SUBST(BOOST_CPPFLAGS)
        AC_SUBST(BOOST_LDFLAGS)
        AC_DEFINE(HAVE_BOOST,,[define if the Boost library is available])
        # execute ACTION-IF-FOUND (if present):
        ifelse([$2], , :, [$2])
    fi

    CPPFLAGS="$CPPFLAGS_SAVED"
    LDFLAGS="$LDFLAGS_SAVED"
fi

])


#############################################################################################
# Check for Boost for Target Architecture 
#############################################################################################

AC_DEFUN([AC_PKG_TARGET_BOOST], [

    AC_ARG_WITH(target-boost,
                AC_HELP_STRING([--with-target-boost=DIR],
                               [Boost target architecture installation @<:@/opt@:>@]),
                target_boost_dir=$withval, target_boost_dir="/zzz")

    AC_MSG_CHECKING([for Targetted Boost support])

    found_target_boost=0
    if test -f $target_boost_dir/$abi_libdir/libboost_system.so -o -f $target_boost_dir/$abi_libdir/libboost_system.a -o -f $target_boost_dir/$abi_libdir/libboost_system.so -o -f $target_boost_dir/$abi_libdir/libboost_system.a; then
       found_target_boost=1
       TARGET_BOOST_LDFLAGS="-L$target_boost_dir/$abi_libdir"
       TARGET_BOOST_LIB="$target_boost_dir/$abi_libdir"
    elif test -f $target_boost_dir/$alt_abi_libdir/libboost_system.so -o -f $target_boost_dir/$alt_abi_libdir/libboost_system.a -o -f $target_boost_dir/$alt_abi_libdir/libboost_system.so -o -f $target_boost_dir/$alt_abi_libdir/libboost_system.a; then
       found_target_boost=1
       TARGET_BOOST_LDFLAGS="-L$target_boost_dir/$alt_abi_libdir"
       TARGET_BOOST_LIB="$target_boost_dir/$abi_libdir"
    fi

    if test $found_target_boost == 0 && test "$target_boost_dir" == "/zzz" ; then
      AM_CONDITIONAL(HAVE_TARGET_BOOST, false)
      TARGET_BOOST_CPPFLAGS=""
      TARGET_BOOST_LDFLAGS=""
      TARGET_BOOST_LIBS=""
      TARGET_BOOST_LIB=""
      TARGET_BOOST_DIR=""
      AC_MSG_RESULT(no)
    elif test $found_target_boost == 1 ; then
      AC_MSG_RESULT(yes)
      AM_CONDITIONAL(HAVE_TARGET_BOOST, true)
      AC_DEFINE(HAVE_TARGET_BOOST, 1, [Define to 1 if you have a target version of BOOST.])
      TARGET_BOOST_CPPFLAGS="-I$target_boost_dir/include/boost -I$target_boost_dir/include"
      TARGET_BOOST_LIBS=""
      TARGET_BOOST_LIB=""
      TARGET_BOOST_DIR="$target_boost_dir"
    else 
      AM_CONDITIONAL(HAVE_TARGET_BOOST, false)
      TARGET_BOOST_CPPFLAGS=""
      TARGET_BOOST_LDFLAGS=""
      TARGET_BOOST_LIBS=""
      TARGET_BOOST_LIB=""
      TARGET_BOOST_DIR=""
      AC_MSG_RESULT(no)
    fi

    AC_ARG_WITH(target-boost-libdir,
                AC_HELP_STRING([--with-target-boost-libdir=DIR],
                               [Boost target architecture installation @<:@/opt@:>@]),
                target_boost_lib_dir=$withval, target_boost_lib_dir="/zzz")

    AC_MSG_CHECKING([for Targetted Boost libdir support])

    if test $found_target_boost == 1 && test "$target_boost_dir" != "/zzz" && test "$target_boost_lib_dir" != "/zzz" ; then
      AC_MSG_RESULT(yes)
    else
      AC_MSG_RESULT(no)
      TARGET_BOOST_LIB=""
    fi

    AC_SUBST(TARGET_BOOST_LIB)
    AC_SUBST(TARGET_BOOST_CPPFLAGS)
    AC_SUBST(TARGET_BOOST_LDFLAGS)
    AC_SUBST(TARGET_BOOST_LIBS)
    AC_SUBST(TARGET_BOOST_DIR)

])
