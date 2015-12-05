
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

#ifdef ZTS
#include "TSRM.h"
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

PHP_METHOD(ykloger, resetRequestTime);
PHP_METHOD(ykloger, getRequestId);

#define YKLOGER_LEVEL_DEBUG  (1<<0L)
#define YKLOGER_LEVEL_INFO  (1<<1L)
#define YKLOGER_LEVEL_TRACE (1<<1L)
#define YKLOGER_LEVEL_WARN   (1<<2L)
#define YKLOGER_LEVEL_ERROR   (1<<3L)
#define YKLOGER_LEVEL_FATAL  (1<<4L)

#define LOGS_POOL_TYPE (1)
#define LOGS_POOL_WF_TYPE (2)

#define YKLOGER_LEVEL_DEBUG_NAME  "DEBUG"
#define YKLOGER_LEVEL_INFO_NAME   "INFO"
#define YKLOGER_LEVEL_TRACE_NAME   "TRACE"
#define YKLOGER_LEVEL_WARN_NAME   "WARN"
#define YKLOGER_LEVEL_ERROR_NAME  "ERROR"
#define YKLOGER_LEVEL_FATAL_NAME  "FATAL"

#define YKLOGER_LOG_INITED "_inited"
#define YKLOGER_LOGFILE_NAME "logFile"
#define YKLOGER_LOGLEVEL_NAME "logLevel"
#define YKLOGER_REQUEST_ID "request_id"
#define YKLOGER_REQUEST_START_TIME  "request_start_time"
#define YKLOGER_POOL_LOG "_pool"
#define YKLOGER_POOL_WF_LOG "_wf_pool"


#define YKLOGER_ALIAS_BEEQUICK_NAME "BqLogger"

#define YKLOGER_GE_WARN_PRE_NAME "wf."

#define MICRO_IN_SEC 1000000.00

#define MAX_PARAMS_STR_LEN (255)
#define MAX_ROW_LOG_STR_LEN (1024)

#ifdef ZTS
#include "TSRM.h"
#endif


ZEND_BEGIN_MODULE_GLOBALS(ykloger)
    zend_long auto_flush_num;
ZEND_END_MODULE_GLOBALS(ykloger)


/* Always refer to the globals in your function as YKLOGER_G(variable).
   You are encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/
#define YKLOGER_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(ykloger, v)

#if defined(ZTS) && defined(COMPILE_DL_YKLOGER)
ZEND_TSRMLS_CACHE_EXTERN();
#endif

#endif	/* PHP_YKLOGER_H */

