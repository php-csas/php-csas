dnl $Id$

PHP_ARG_ENABLE(csas, whether to enable csas support,
[  --enable-csas           Enable csas support])

if test "$PHP_CSAS" != "no"; then
  PHP_NEW_EXTENSION(csas, csas.c formatted_csas_print.c htmlparser/htmlparser.c htmlparser/jsparser.c htmlparser/statemachine.c sanitizers/sanitizers.c, $ext_shared)
fi
