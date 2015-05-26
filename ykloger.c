
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "ext/date/php_date.h"
#include "ext/standard/md5.h"
#include "php_ykloger.h"

/* If you declare any globals in php_ykloger.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(ykloger)
*/

/* True global resources - no need for thread safety here */
static int le_ykloger;

//reqest start time
static double request_start_time;
static char* request_id;

zend_class_entry * ykloger_ce;

ZEND_BEGIN_ARG_INFO(arginfo_ykloger_init, 0)
    ZEND_ARG_ARRAY_INFO(0, config, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ykloger_write, 0, 0, 1)
    ZEND_ARG_INFO(0, message)
    ZEND_ARG_INFO(0, errno)
    ZEND_ARG_ARRAY_INFO(0, params, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_reset_request_time, 0, 0, 0)
    ZEND_ARG_INFO(0, time)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_get_request_id, 0)
ZEND_END_ARG_INFO()

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("ykloger.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_ykloger_globals, ykloger_globals)
    STD_PHP_INI_ENTRY("ykloger.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_ykloger_globals, ykloger_globals)
PHP_INI_END()
*/
/* }}} */


static zend_function_entry ykloger_methods[] = {
    PHP_ME(ykloger, init, arginfo_ykloger_init, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(ykloger, debug, arginfo_ykloger_write, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(ykloger, info, arginfo_ykloger_write, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_MALIAS(ykloger, trace, info, arginfo_ykloger_write, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(ykloger, warn, arginfo_ykloger_write, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(ykloger, error, arginfo_ykloger_write, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(ykloger, fatal, arginfo_ykloger_write, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(ykloger, reset_request_time, arginfo_reset_request_time, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_MALIAS(ykloger, resetStartTime, reset_request_time, arginfo_reset_request_time, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(ykloger, get_request_id, arginfo_get_request_id, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_MALIAS(ykloger, getRequestId, get_request_id, arginfo_get_request_id, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_FE_END
};

#define LOGER_LEVEL_ENABLE(level) \
    do{ \
        zval *loger_level; \
        loger_level = zend_read_static_property(ykloger_ce, ZEND_STRL(YKLOGER_LOGLEVEL_NAME), 1 TSRMLS_DC);\
        if (level < Z_LVAL_P(loger_level)) { \
            RETURN_NULL(); \
        } \
    }while(false);\

#define EXIST_EFREE(a) \
    if(a){\
        efree(a);\
    }\

PHP_METHOD(ykloger, reset_request_time){
    uint request_time = 0;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &request_time) == FAILURE) {
        RETURN_FALSE;
    }
    request_start_time = request_time ? request_time : time(NULL);
    RETURN_TRUE;
}

PHP_METHOD(ykloger, get_request_id){
    RETURN_STRING(request_id, 1);
}

static char * get_request_server(char *name, uint len TSRMLS_DC){
    zval **args = NULL;
    
    if ((PG(http_globals)[TRACK_VARS_SERVER] || zend_is_auto_global(ZEND_STRL("_SERVER") TSRMLS_CC)) && (zend_hash_find(Z_ARRVAL_P((PG(http_globals))[TRACK_VARS_SERVER]), name, len, (void **) &args) != FAILURE || zend_hash_find(&EG(symbol_table), name, len, (void **) &args) != FAILURE)){
        
        if (!args || !(*args)) {
            return NULL;
        }
        
        return estrndup(Z_STRVAL_PP(args), Z_STRLEN_PP(args));
    }
    
    return NULL;
}

static char * ykloger_array_to_string(char **file, uint *line, zval *params TSRMLS_DC){
    HashTable *pmht;
    char *str=NULL;
    uint str_start = 0, str_length = MAX_PARAMS_STR_LEN;
    
    str = (char *)emalloc(str_length + 1);
    
    pmht = Z_ARRVAL_P(params);
    for (zend_hash_internal_pointer_reset(pmht); zend_hash_has_more_elements(pmht) == SUCCESS; zend_hash_move_forward(pmht)) {
        char *key=NULL, *tmp=NULL;
        ulong idx, tmp_len;
        int type;
        zval **ppzval, tmpcopy;
        
        type = zend_hash_get_current_key(pmht, &key, &idx, 0);
        
        if (type == HASH_KEY_NON_EXISTANT) {
            continue;
        }
        
        if (zend_hash_get_current_data(pmht, (void**)&ppzval) == FAILURE) {
            continue;
        }
        
        tmpcopy = **ppzval;
        zval_copy_ctor(&tmpcopy);
        INIT_PZVAL(&tmpcopy);
        
        if (type == HASH_KEY_IS_LONG) {
            convert_to_string(&tmpcopy);
            //php_printf("%ld[%s] \n", idx, Z_STRVAL(tmpcopy));
            tmp_len = spprintf(&tmp, 0, "%ld[%s] ", idx, Z_STRVAL(tmpcopy));
        }else{
            
            if (strcmp(key, "CUSTOM_LINE") == 0) {
                convert_to_long(&tmpcopy);
                *line = Z_LVAL(tmpcopy);
                zval_dtor(&tmpcopy);
                continue;
            }
            
            convert_to_string(&tmpcopy);
            if (strcmp(key, "CUSTOM_FILE") == 0) {
                *file = estrndup(Z_STRVAL(tmpcopy), Z_STRLEN(tmpcopy));
                zval_dtor(&tmpcopy);
                continue;
            }
            
            //php_printf("%s[%s] \n", idx, Z_STRVAL(tmpcopy));
            tmp_len = spprintf(&tmp, 0, "%s[%s] ", key, Z_STRVAL(tmpcopy));
        }
        zval_dtor(&tmpcopy);
        
        if (str_start + tmp_len + 1 > str_length) {
            str_length += MAX_PARAMS_STR_LEN;
            str = (char *) erealloc(str, str_length);
        }
        
        memcpy(str + str_start, tmp, tmp_len);
        efree(tmp);
        
        str_start += tmp_len;
    }
    *(str + str_start) = '\0';
    
    return str;
}

static void ykloger_write(uint level_num, const char *level, const char * file_name_pre, char *message, uint message_len, uint loger_errno, zval *params TSRMLS_DC){
    char *file=NULL;
    uint line=0;
    zval *logfile;
    char *file_name, *cur_date, *true_file;
    struct timeval tp = {0};
    char *ip=NULL, *uri=NULL, *refer=NULL, *params_str=NULL;
    double cost_usec;
    pid_t cur_pid;
    char *str_message=NULL;
    uint str_message_len;
    php_stream *stream;
    
    logfile = zend_read_static_property(ykloger_ce, ZEND_STRL(YKLOGER_LOGFILE_NAME), 1 TSRMLS_CC);
    
    if (!logfile) {
        php_error(E_WARNING, "ykloger undeclared log file");
        return;
    }
    
    gettimeofday(&tp, NULL);
    
    cur_pid = getpid();

    uri = get_request_server(ZEND_STRS("REQUEST_URI") TSRMLS_CC);
    ip = get_request_server(ZEND_STRS("REMOTE_ADDR") TSRMLS_CC);
    refer = get_request_server(ZEND_STRS("HTTP_REFERER") TSRMLS_CC);
    
    file_name = php_format_date("YmdH", sizeof("YmdH"), time(NULL), 1 TSRMLS_CC);
    cur_date = php_format_date("Y-m-d H:i:s", sizeof("Y-m-d H:i:s"), time(NULL), 1 TSRMLS_CC);
    
    cost_usec = tp.tv_sec + tp.tv_usec / MICRO_IN_SEC - request_start_time;
    
    params_str = params ? ykloger_array_to_string(&file, &line, params TSRMLS_CC) : NULL;
    
    if (!file || line == 0) {
        file = zend_get_executed_filename(TSRMLS_C);
        line = zend_get_executed_lineno(TSRMLS_C);
    }
    
    str_message_len = spprintf(&str_message, 0, "%-5s %s,%d [%s:%d] [%d] reqip[%s] uri[%s] refer[%s] reqid[%s], cost[%f] error[%d] %s%s\n", level, cur_date, tp.tv_usec, file, line, cur_pid, ip ? ip : "", uri ? uri : "", refer ? refer : "", request_id, cost_usec, loger_errno, params_str ? params_str : "", message);
    
    EXIST_EFREE(params_str);
    EXIST_EFREE(ip);
    EXIST_EFREE(uri);
    EXIST_EFREE(refer);
    
    true_file = (char*) emalloc(MAXPATHLEN);
    sprintf(true_file, "%s.%s%s", Z_STRVAL_P(logfile), level_num >= YKLOGER_LEVEL_WARN ? YKLOGER_GE_WARN_PRE_NAME : "", file_name);
    
    stream = php_stream_open_wrapper(true_file, "ab", USE_PATH | REPORT_ERRORS, NULL);
    if (stream == NULL) {
        php_error(E_WARNING, "log file open fail %s", true_file);
        efree(str_message);
        efree(true_file);
        return;
    }
    
    php_stream_write(stream, str_message, str_message_len);
    php_stream_close(stream);
    
    efree(str_message);
    efree(true_file);
}

PHP_METHOD(ykloger, init){
    zval *conf;
    zval **logfile, **loglevel;
    uint level_val;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a", &conf) == FAILURE) {
        RETURN_FALSE;
    }
    
    if (zend_hash_find(Z_ARRVAL_P(conf), ZEND_STRS(YKLOGER_LOGFILE_NAME), (void **)&logfile) == FAILURE) {
        php_error(E_ERROR, "ykloger init undeclared %s", YKLOGER_LOGFILE_NAME);
        RETURN_FALSE;
    }
    
    if (zend_hash_find(Z_ARRVAL_P(conf), ZEND_STRS(YKLOGER_LOGLEVEL_NAME), (void **)&loglevel) == FAILURE) {
        php_error(E_ERROR, "ykloger init undeclared %s", YKLOGER_LOGLEVEL_NAME);
        RETURN_FALSE;
    }
    
    if (Z_TYPE_PP(loglevel) == IS_STRING) {
        char *level_str;
        level_str = estrndup(Z_STRVAL_PP(loglevel), Z_STRLEN_PP(loglevel));
        php_strtoupper(level_str, Z_STRLEN_PP(loglevel));
        if (strcmp(level_str, YKLOGER_LEVEL_DEBUG_NAME) == 0) {
            level_val = YKLOGER_LEVEL_DEBUG;
        }else if (strcmp(level_str, YKLOGER_LEVEL_INFO_NAME) == 0 || strcmp(level_str, YKLOGER_LEVEL_TRACE_NAME) == 0) {
            level_val = YKLOGER_LEVEL_INFO;
        }else if (strcmp(level_str, YKLOGER_LEVEL_WARN_NAME) == 0) {
            level_val = YKLOGER_LEVEL_WARN;
        }else if (strcmp(level_str, YKLOGER_LEVEL_ERROR_NAME) == 0) {
            level_val = YKLOGER_LEVEL_ERROR;
        }else if (strcmp(level_str, YKLOGER_LEVEL_FATAL_NAME) == 0) {
            level_val = YKLOGER_LEVEL_FATAL;
        }else{
            php_error(E_ERROR, "ykloger init invalid logLevel name %s", level_str);
        }
        efree(level_str);
    }else{
        convert_to_long_ex(loglevel);
        level_val = Z_LVAL_PP(loglevel);
    }
    
    zend_update_static_property(ykloger_ce, ZEND_STRL(YKLOGER_LOGFILE_NAME), *logfile TSRMLS_CC);
    zend_update_static_property_long(ykloger_ce, ZEND_STRL(YKLOGER_LOGLEVEL_NAME), level_val TSRMLS_CC);

    RETURN_TRUE;
}

PHP_METHOD(ykloger, debug){
    char *message;
    char path[MAXPATHLEN];
    uint message_len, loger_errno = 0;
    zval *params = NULL;
    
    LOGER_LEVEL_ENABLE(YKLOGER_LEVEL_DEBUG);
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|la", &message, &message_len, &loger_errno, &params) == FAILURE) {
        RETURN_FALSE;
    }
    
    ykloger_write(YKLOGER_LEVEL_DEBUG, YKLOGER_LEVEL_DEBUG_NAME, "", message, message_len, loger_errno, params TSRMLS_CC);
    
    RETURN_TRUE;
}

PHP_METHOD(ykloger, info){
    char *message;
    uint message_len, loger_errno = 0;
    zval *params = NULL;
    
    LOGER_LEVEL_ENABLE(YKLOGER_LEVEL_INFO);
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|la", &message, &message_len, &loger_errno, &params) == FAILURE) {
        RETURN_FALSE;
    }
    
    ykloger_write(YKLOGER_LEVEL_INFO, YKLOGER_LEVEL_INFO_NAME, "", message, message_len, loger_errno, params TSRMLS_CC);
    
    RETURN_TRUE;
}

PHP_METHOD(ykloger, warn){
    char *message;
    uint message_len, loger_errno = 0;
    zval *params = NULL;
    
    LOGER_LEVEL_ENABLE(YKLOGER_LEVEL_WARN);
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|la", &message, &message_len, &loger_errno, &params) == FAILURE) {
        RETURN_FALSE;
    }
    
    ykloger_write(YKLOGER_LEVEL_WARN, YKLOGER_LEVEL_WARN_NAME, "wf", message, message_len, loger_errno, params TSRMLS_CC);
    
    RETURN_TRUE;
}

PHP_METHOD(ykloger, error){
    char *message;
    uint message_len, loger_errno = 0;
    zval *params = NULL;
    
    LOGER_LEVEL_ENABLE(YKLOGER_LEVEL_ERROR);
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|la", &message, &message_len, &loger_errno, &params) == FAILURE) {
        RETURN_FALSE;
    }
    
    ykloger_write(YKLOGER_LEVEL_ERROR, YKLOGER_LEVEL_ERROR_NAME, "wf", message, message_len, loger_errno, params TSRMLS_CC);
    
    RETURN_TRUE;
}

PHP_METHOD(ykloger, fatal){
    char *message;
    uint message_len, loger_errno = 0;
    zval *params = NULL;
    
    LOGER_LEVEL_ENABLE(YKLOGER_LEVEL_FATAL);
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|la", &message, &message_len, &loger_errno, &params) == FAILURE) {
        RETURN_FALSE;
    }
    
    ykloger_write(YKLOGER_LEVEL_FATAL, YKLOGER_LEVEL_FATAL_NAME, "wf", message, message_len, loger_errno, params TSRMLS_CC);
    
    RETURN_TRUE;
}


/* The previous line is meant for vim and emacs, so it can correctly fold and 
   unfold functions in source code. See the corresponding marks just before 
   function definition, where the functions purpose is also documented. Please 
   follow this convention for the convenience of others editing your code.
*/


/* {{{ php_ykloger_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_ykloger_init_globals(zend_ykloger_globals *ykloger_globals)
{
	ykloger_globals->global_value = 0;
	ykloger_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(ykloger)
{
	/* If you have INI entries, uncomment these lines 
	REGISTER_INI_ENTRIES();
	*/
    zend_register_long_constant("YKLOGER_LEVEL_DEBUG", sizeof("YKLOGER_LEVEL_DEBUG"), YKLOGER_LEVEL_DEBUG, CONST_CS | CONST_PERSISTENT, module_number TSRMLS_DC);
    zend_register_long_constant("YKLOGER_LEVEL_INFO", sizeof("YKLOGER_LEVEL_INFO"), YKLOGER_LEVEL_INFO, CONST_CS | CONST_PERSISTENT, module_number TSRMLS_DC);
    zend_register_long_constant("YKLOGER_LEVEL_WARN", sizeof("YKLOGER_LEVEL_WARN"), YKLOGER_LEVEL_WARN, CONST_CS | CONST_PERSISTENT, module_number TSRMLS_DC);
    zend_register_long_constant("YKLOGER_LEVEL_ERROR", sizeof("YKLOGER_LEVEL_ERROR"), YKLOGER_LEVEL_ERROR, CONST_CS | CONST_PERSISTENT, module_number TSRMLS_DC);
    zend_register_long_constant("YKLOGER_LEVEL_FATAL", sizeof("YKLOGER_LEVEL_FATAL"), YKLOGER_LEVEL_FATAL, CONST_CS | CONST_PERSISTENT, module_number TSRMLS_DC);
    
    zend_class_entry ce;
    
    INIT_CLASS_ENTRY(ce, "ykloger", ykloger_methods);
    ykloger_ce = zend_register_internal_class(&ce TSRMLS_CC);
    
    zend_declare_property_null(ykloger_ce, ZEND_STRL(YKLOGER_LOGFILE_NAME), ZEND_ACC_STATIC | ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_null(ykloger_ce, ZEND_STRL(YKLOGER_LOGLEVEL_NAME), ZEND_ACC_STATIC | ZEND_ACC_PUBLIC TSRMLS_CC);
    
    zend_register_class_alias(yKLOGER_ALIAS_BEEQUICK_NAME, ykloger_ce);
    
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(ykloger)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(ykloger)
{
    struct timeval tp = {0};
    PHP_MD5_CTX context;
    unsigned char digest[16];
    char *arg;
    uint arg_len;

    gettimeofday(&tp, NULL);
    request_start_time = tp.tv_sec + tp.tv_usec / MICRO_IN_SEC;
    
    request_id = (char *) emalloc(33);
    
    arg_len = spprintf(&arg, 0, "%s%08x%05x%.8F", "ykloger", (int)tp.tv_sec, (int)(tp.tv_usec % 0x100000), php_combined_lcg(TSRMLS_C) * 10);
    
    PHP_MD5Init(&context);
    PHP_MD5Update(&context, arg, arg_len);
    PHP_MD5Final(digest, &context);
    make_digest_ex(request_id, digest, 16);
    
    *(request_id + 33) = '\0';
    
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(ykloger)
{
    request_start_time = 0;
    if (request_id) {
        efree(request_id);
        request_id = NULL;
    }
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(ykloger)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "ykloger support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */

/* {{{ ykloger_functions[]
 *
 * Every user visible function must have an entry in ykloger_functions[].
 */
const zend_function_entry ykloger_functions[] = {
	PHP_FE_END	/* Must be the last line in ykloger_functions[] */
};
/* }}} */

/* {{{ ykloger_module_entry
 */
zend_module_entry ykloger_module_entry = {
	STANDARD_MODULE_HEADER,
	"ykloger",
	ykloger_functions,
	PHP_MINIT(ykloger),
	PHP_MSHUTDOWN(ykloger),
	PHP_RINIT(ykloger),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(ykloger),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(ykloger),
	PHP_YKLOGER_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_YKLOGER
ZEND_GET_MODULE(ykloger)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
