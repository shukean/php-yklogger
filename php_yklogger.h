
#ifndef PHP_YKLOGGER_H
#define PHP_YKLOGGER_H

extern zend_module_entry yklogger_module_entry;
#define phpext_yklogger_ptr &yklogger_module_entry

#define PHP_YKLOGGER_VERSION "0.1.0" /* Replace with version number for your extension */

#ifdef PHP_WIN32
#	define PHP_YKLOGGER_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_YKLOGGER_API __attribute__ ((visibility("default")))
#else
#	define PHP_YKLOGGER_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(yklogger);
PHP_MSHUTDOWN_FUNCTION(yklogger);

PHP_RINIT_FUNCTION(yklogger);
PHP_RSHUTDOWN_FUNCTION(yklogger);

extern zend_class_entry * yklogger_ce;

PHP_METHOD(yklogger, init);
PHP_METHOD(yklogger, debug);
PHP_METHOD(yklogger, info);
PHP_METHOD(yklogger, warn);
PHP_METHOD(yklogger, error);
PHP_METHOD(yklogger, fatal);

PHP_METHOD(yklogger, resetRequestTime);
PHP_METHOD(yklogger, getRequestId);

#define YKLOGGER_LEVEL_DEBUG  (1<<0L)
#define YKLOGGER_LEVEL_INFO  (1<<1L)
#define YKLOGGER_LEVEL_TRACE (1<<1L)
#define YKLOGGER_LEVEL_WARN   (1<<2L)
#define YKLOGGER_LEVEL_ERROR   (1<<3L)
#define YKLOGGER_LEVEL_FATAL  (1<<4L)

#define LOGS_POOL_TYPE (1)
#define LOGS_POOL_WF_TYPE (2)

#define YKLOGGER_LEVEL_DEBUG_NAME  "DEBUG"
#define YKLOGGER_LEVEL_INFO_NAME   "INFO"
#define YKLOGGER_LEVEL_TRACE_NAME   "TRACE"
#define YKLOGGER_LEVEL_WARN_NAME   "WARN"
#define YKLOGGER_LEVEL_ERROR_NAME  "ERROR"
#define YKLOGGER_LEVEL_FATAL_NAME  "FATAL"

#define YKLOGGER_LOG_INITED "_inited"
#define YKLOGGER_LOGFILE_NAME "logFile"
#define YKLOGGER_LOGLEVEL_NAME "logLevel"
#define YKLOGGER_REQUEST_ID "request_id"
#define YKLOGGER_REQUEST_START_TIME  "request_start_time"
#define YKLOGGER_POOL_LOG "_pool"
#define YKLOGGER_POOL_WF_LOG "_wf_pool"

#define YKLOGGER_GE_WARN_PRE_NAME "wf."

#define MICRO_IN_SEC 1000000.00

#define MAX_PARAMS_STR_LEN (255)
#define MAX_ROW_LOG_STR_LEN (1024)

#ifdef ZTS
#include "TSRM.h"
#endif


ZEND_BEGIN_MODULE_GLOBALS(yklogger)
    zend_long auto_flush_num;
ZEND_END_MODULE_GLOBALS(yklogger)


/* Always refer to the globals in your function as YKLOGGER_G(variable).
   You are encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/
#define YKLOGGER_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(yklogger, v)

#if defined(ZTS) && defined(COMPILE_DL_YKLOGER)
ZEND_TSRMLS_CACHE_EXTERN();
#endif

#endif	/* PHP_YKLOGGER_H */

