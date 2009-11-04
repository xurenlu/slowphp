dnl $Id$
dnl config.m4 for extension slowphp

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

PHP_ARG_WITH(slowphp, for slowphp support,
dnl Make sure that the comment is aligned:
[  --with-slowphp             Include slowphp support])

dnl Otherwise use enable:

dnl PHP_ARG_ENABLE(slowphp, whether to enable slowphp support,
dnl Make sure that the comment is aligned:
dnl [  --enable-slowphp           Enable slowphp support])

if test "$PHP_SLOWPHP" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-slowphp -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/slowphp.h"  # you most likely want to change this
  dnl if test -r $PHP_SLOWPHP/$SEARCH_FOR; then # path given as parameter
  dnl   SLOWPHP_DIR=$PHP_SLOWPHP
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for slowphp files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       SLOWPHP_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$SLOWPHP_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the slowphp distribution])
  dnl fi

  dnl # --with-slowphp -> add include path
  dnl PHP_ADD_INCLUDE($SLOWPHP_DIR/include)

  dnl # --with-slowphp -> check for lib and symbol presence
  dnl LIBNAME=slowphp # you may want to change this
  dnl LIBSYMBOL=slowphp # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $SLOWPHP_DIR/lib, SLOWPHP_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_SLOWPHPLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong slowphp lib version or lib not found])
  dnl ],[
  dnl   -L$SLOWPHP_DIR/lib -lm -ldl
  dnl ])
  dnl
  dnl PHP_SUBST(SLOWPHP_SHARED_LIBADD)

  PHP_NEW_EXTENSION(slowphp, slowphp.c, $ext_shared)
fi
