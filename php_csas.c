// include PHP API
#include <php.h>
 
// header file we'll create below
#include "php_csas.h"
 
// define the function(s) we want to add
zend_function_entry csas_functions[] = {
  PHP_FE(csas_main, NULL)
  { NULL, NULL, NULL }
};
 
// "csas_functions" refers to the struct defined above
// we'll be filling in more of this later: you can use this to specify
// globals, php.ini info, startup and teardown functions, etc.
zend_module_entry csas_module_entry = {
  STANDARD_MODULE_HEADER,
  PHP_CSAS_EXTNAME,
  csas_functions,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  PHP_CSAS_VERSION,
  STANDARD_MODULE_PROPERTIES
};
 
// install module
ZEND_GET_MODULE(csas)
 
// actual non-template code!
PHP_FUNCTION(csas_main) {
    // php_printf is PHP's version of printf, it's essentially "echo" from C
    php_printf("Hello, CSAS!\n");
}
