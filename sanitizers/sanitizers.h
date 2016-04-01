/*
 +----------------------------------------------------------------------+
  | CSAS                                                                |
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

/*Sanitizer declarations*/
char* html_escape_sanitize(const char* op1, int len);
char* pre_escape_sanitize(const char* op1, int len);
char* html_unquoted(const char* op1, int len);
char* javascript_escape_sanitize(const char* op1, int len);
char* url_query_escape_sanitize(const char* op1, int len);
char *url_start_sanitize(const char* op1, int len);
char *url_general_sanitize(const char* op1, int len);
