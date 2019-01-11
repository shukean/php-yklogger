dnl $Id$
dnl config.m4 for extension yklogger

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(yklogger, for yklogger support,
dnl Make sure that the comment is aligned:
dnl [  --with-yklogger             Include yklogger support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(yklogger, whether to enable yklogger support,
Make sure that the comment is aligned:
[  --enable-yklogger           Enable yklogger support])

if test "$PHP_YKLOGGER" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-yklogger -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/yklogger.h"  # you most likely want to change this
  dnl if test -r $PHP_YKLOGGER/$SEARCH_FOR; then # path given as parameter
  dnl   YKLOGGER_DIR=$PHP_YKLOGGER
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for yklogger files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       YKLOGGER_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$YKLOGGER_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the yklogger distribution])
  dnl fi

  dnl # --with-yklogger -> add include path
  dnl PHP_ADD_INCLUDE($YKLOGGER_DIR/include)

  dnl # --with-yklogger -> check for lib and symbol presence
  dnl LIBNAME=yklogger # you may want to change this
  dnl LIBSYMBOL=yklogger # you most likely want to change this

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $YKLOGGER_DIR/$PHP_LIBDIR, YKLOGGER_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_YKLOGGERLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong yklogger lib version or lib not found])
  dnl ],[
  dnl   -L$YKLOGGER_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  dnl PHP_SUBST(YKLOGGER_SHARED_LIBADD)

  PHP_NEW_EXTENSION(yklogger, yklogger.c, $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
fi
