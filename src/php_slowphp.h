
#ifndef PHP_slowphp_H
#define PHP_slowphp_H

extern zend_module_entry slowphp_module_entry;
#define phpext_slowphp_ptr &slowphp_module_entry

#ifdef PHP_WIN32
#define PHP_slowphp_API __declspec(dllexport)
#else
#define PHP_slowphp_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(slowphp);
PHP_MSHUTDOWN_FUNCTION(slowphp);
PHP_RINIT_FUNCTION(slowphp);
PHP_RSHUTDOWN_FUNCTION(slowphp);
PHP_MINFO_FUNCTION(slowphp);

PHP_FUNCTION(confirm_slowphp_compiled);	/* For testing, remove later. */
PHP_FUNCTION(slowphp_get_long_query_time);
PHP_FUNCTION(slowphp_get_long_query_log);
PHP_FUNCTION(slowphp_get_query_log_probability);
PHP_FUNCTION(slowphp_get_query_log_lock_file);
PHP_FUNCTION(slowphp_get_magic_line);
PHP_FUNCTION(slowphp_get_long_query_uri_pattern);

/* 
  	Declare any global variables you may need between the BEGIN
	and END macros here:     
*/
ZEND_BEGIN_MODULE_GLOBALS(slowphp)
	long  long_query_time;
	char *long_query_log;
    long long_query_log_probability;
    char * long_query_lock_file;
    char * magic_line;
    char * long_query_uri_pattern;
ZEND_END_MODULE_GLOBALS(slowphp)

//*/

/* In every utility function you add that needs to use variables 
   in php_slowphp_globals, call TSRMLS_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as slowphp_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define slowphp_G(v) TSRMG(slowphp_globals_id, zend_slowphp_globals *, v)
#else
#define slowphp_G(v) (slowphp_globals.v)
#endif

#endif	/* PHP_slowphp_H */

