dnl $Id$

PHP_ARG_ENABLE(csas, whether to enable csas support,
[  --enable-csas           Enable csas support])

if test "$PHP_TAINT" != "no"; then
  PHP_NEW_EXTENSION(taint, taint.c, $ext_shared)
fi
