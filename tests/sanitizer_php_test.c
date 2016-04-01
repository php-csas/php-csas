/*
  +----------------------------------------------------------------------+
  | Sanitizer PHP Functions                                                                 |
  +----------------------------------------------------------------------+
  | Copyright (c) 2012-2015 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:  Matt Van Gundy    <mvangund@cisco.com>                      |
  +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sanitizers/sanitizers.h"
#include "php.h"
#include "php_csas.h"
#include "php_sanitizers.h"

char* cast_zval_to_string(zval *z) {
  if (Z_TYPE_P(z) != IS_STRING) {
      return NULL;
  }
  return Z_STRVAL_P(z);
}

/*function run sanitier from php*/
PHP_FUNCTION(html_sanitizer){
  zval **arg;
  char *op1;
  int len;
  uint safety = PHP_CSAS_SAFE_ALL;
  if (ZEND_NUM_ARGS() != 1 || zend_get_parameters_ex(1, &arg) == FAILURE) {
    WRONG_PARAM_COUNT;
  }
  len = Z_STRLEN(arg);
  op1 = cast_zval_to_string(arg);
  op1 = html_escape_sanitize(op1, len);
  php_printf("%s\n", op1);
  free(op1);
}

PHP_FUNCTION(js_sanitizer){
 zval **arg;
  char *op1;
  int len;
  uint safety = PHP_CSAS_SAFE_ALL;
  if (ZEND_NUM_ARGS() != 1 || zend_get_parameters_ex(1, &arg) == FAILURE) {
    WRONG_PARAM_COUNT;
  }
  len = Z_STRLEN(arg);
  op1 = cast_zval_to_string(arg);
  op1 = js_escape_sanitize(op1, len);
  php_printf("%s\n", op1);
  free(op1);
}

PHP_FUNCTION(url_querry_sanitizer){
  zval **arg;
  char *op1;
  int len;
  uint safety = PHP_CSAS_SAFE_ALL;
  if (ZEND_NUM_ARGS() != 1 || zend_get_parameters_ex(1, &arg) == FAILURE) {
    WRONG_PARAM_COUNT;
  }
  len = Z_STRLEN(arg);
  op1 = cast_zval_to_string(arg);
  op1 = url_query_escape_sanitize(op1, len);
  php_printf("%s\n", op1);
  free(op1);
}

PHP_FUNCTION(url_start_sanitizer){
  zval **arg;
  char *op1;
  int len;
  uint safety = PHP_CSAS_SAFE_ALL;
  if (ZEND_NUM_ARGS() != 1 || zend_get_parameters_ex(1, &arg) == FAILURE) {
    WRONG_PARAM_COUNT;
  }
  len = Z_STRLEN(arg);
  op1 = cast_zval_to_string(arg);
  op1 = url_start_sanitize(op1, len);
  php_printf("%s\n", op1);
  free(op1);
}

PHP_FUNCTION(url_general_sanitizer){
  zval **arg;
  char *op1;
  int len;
  uint safety = PHP_CSAS_SAFE_ALL;
  if (ZEND_NUM_ARGS() != 1 || zend_get_parameters_ex(1, &arg) == FAILURE) {
    WRONG_PARAM_COUNT;
  }
  len = Z_STRLEN(arg);
  op1 = cast_zval_to_string(arg);
  op1 = url_general_sanitize(op1, len);
  php_printf("%s\n", op1);
  free(op1);
}

PHP_FUNCTION(pre_escape_sanitizer){
  zval **arg;
  char *op1;
  int len;
  uint safety = PHP_CSAS_SAFE_ALL;
  if (ZEND_NUM_ARGS() != 1 || zend_get_parameters_ex(1, &arg) == FAILURE) {
    WRONG_PARAM_COUNT;
  }
  len = Z_STRLEN(arg);
  op1 = cast_zval_to_string(arg);
  op1 = pre_escape_sanitize(op1, len);
  php_printf("%s\n", op1);
  free(op1);
}