dnl $Id$
dnl config.m4 for extension ykloger

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(ykloger, for ykloger support,
dnl Make sure that the comment is aligned:
dnl [  --with-ykloger             Include ykloger support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(ykloger, whether to enable ykloger support,
Make sure that the comment is aligned:
[  --enable-ykloger           Enable ykloger support])

if test "$PHP_YKLOGER" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-ykloger -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/ykloger.h"  # you most likely want to change this
  dnl if test -r $PHP_YKLOGER/$SEARCH_FOR; then # path given as parameter
  dnl   YKLOGER_DIR=$PHP_YKLOGER
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for ykloger files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       YKLOGER_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$YKLOGER_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the ykloger distribution])
  dnl fi

  dnl # --with-ykloger -> add include path
  dnl PHP_ADD_INCLUDE($YKLOGER_DIR/include)

  dnl # --with-ykloger -> check for lib and symbol presence
  dnl LIBNAME=ykloger # you may want to change this
  dnl LIBSYMBOL=ykloger # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $YKLOGER_DIR/$PHP_LIBDIR, YKLOGER_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_YKLOGERLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong ykloger lib version or lib not found])
  dnl ],[
  dnl   -L$YKLOGER_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  dnl PHP_SUBST(YKLOGER_SHARED_LIBADD)

  PHP_NEW_EXTENSION(ykloger, ykloger.c, $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
fi
