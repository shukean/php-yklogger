
#ifndef PHP_YKLOGER_H
#define PHP_YKLOGER_H

extern zend_module_entry ykloger_module_entry;
#define phpext_ykloger_ptr &ykloger_module_entry

#define PHP_YKLOGER_VERSION "0.1.0" /* Replace with version number for your extension */

#ifdef PHP_WIN32
#	define PHP_YKLOGER_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_YKLOGER_API __attribute__ ((visibility("default")))
#else
#	define PHP_YKLOGER_API
#endif


#define LEVEL_DEBUG     (1<<0L)
#define LEVEL_INFO      (1<<1L)
#define LEVEL_WARN      (1<<2L)
#define LEVEL_ERROR     (1<<3L)


PHP_MINIT_FUNCTION(ykloger);
PHP_MSHUTDOWN_FUNCTION(ykloger);

PHP_RINIT_FUNCTION(ykloger);
PHP_RSHUTDOWN_FUNCTION(ykloger);

extern zend_class_entry * ykloger_ce;

PHP_METHOD(ykloger, init);
PHP_METHOD(ykloger, debug);
PHP_METHOD(ykloger, info);
PHP_METHOD(ykloger, warn);
PHP_METHOD(ykloger, error);
PHP_METHOD(ykloger, fatal);

PHP_METHOD(ykloger, reset_request_time);
PHP_METHOD(ykloger, get_request_id);

#define YKLOGER_LEVEL_DEBUG  (1<<0L)
#define YKLOGER_LEVEL_INFO  (1<<1L)
#define YKLOGER_LEVEL_TRACE (1<<1L)
#define YKLOGER_LEVEL_WARN   (1<<2L)
#define YKLOGER_LEVEL_ERROR   (1<<3L)
#define YKLOGER_LEVEL_FATAL  (1<<4L)

#define YKLOGER_LEVEL_DEBUG_NAME  "DEBUG"
#define YKLOGER_LEVEL_INFO_NAME   "INFO"
#define YKLOGER_LEVEL_TRACE_NAME   "TRACE"
#define YKLOGER_LEVEL_WARN_NAME   "WARN"
#define YKLOGER_LEVEL_ERROR_NAME  "ERROR"
#define YKLOGER_LEVEL_FATAL_NAME  "FATAL"

#define YKLOGER_LOGFILE_NAME "logFile"
#define YKLOGER_LOGLEVEL_NAME "logLevel"

#define yKLOGER_ALIAS_BEEQUICK_NAME "BqLogger"

#define YKLOGER_GE_WARN_PRE_NAME "wf"

#define MICRO_IN_SEC 1000000.00

#define MAX_PARAMS_STR_LEN (255)
#define MAX_ROW_LOG_STR_LEN (102400)

#ifdef ZTS
#include "TSRM.h"
#endif

/* 
  	Declare any global variables you may need between the BEGIN
	and END macros here:     

ZEND_BEGIN_MODULE_GLOBALS(ykloger)
	long  global_value;
	char *global_string;
ZEND_END_MODULE_GLOBALS(ykloger)
*/

/* In every utility function you add that needs to use variables 
   in php_ykloger_globals, call TSRMLS_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as YKLOGER_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define YKLOGER_G(v) TSRMG(ykloger_globals_id, zend_ykloger_globals *, v)
#else
#define YKLOGER_G(v) (ykloger_globals.v)
#endif

#endif	/* PHP_YKLOGER_H */

