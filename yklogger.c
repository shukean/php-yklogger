
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "ext/date/php_date.h"
#include "ext/json/php_json.h"
#include "zend_smart_str.h"
#include "ext/standard/php_string.h"
#include "ext/standard/php_lcg.h"
#include "ext/standard/md5.h"
#include "php_yklogger.h"


ZEND_DECLARE_MODULE_GLOBALS(yklogger)


/* True global resources - no need for thread safety here */
static int le_yklogger;

zend_class_entry * yklogger_ce;

ZEND_BEGIN_ARG_INFO(arginfo_yklogger_init, 0)
ZEND_ARG_ARRAY_INFO(0, config, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_yklogger_write, 0, 0, 1)
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
 PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("yklogger.auto_flush_num",      "50", PHP_INI_ALL, OnUpdateLongGEZero, auto_flush_num, zend_yklogger_globals, yklogger_globals)
 PHP_INI_END()
/* }}} */

static zend_function_entry yklogger_methods[] = {
    PHP_ME(yklogger, init, arginfo_yklogger_init, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(yklogger, debug, arginfo_yklogger_write, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(yklogger, info, arginfo_yklogger_write, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_MALIAS(yklogger, trace, info, arginfo_yklogger_write, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(yklogger, warn, arginfo_yklogger_write, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(yklogger, error, arginfo_yklogger_write, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(yklogger, fatal, arginfo_yklogger_write, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(yklogger, resetRequestTime, arginfo_reset_request_time, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_MALIAS(yklogger, reset_request_time, resetRequestTime, arginfo_reset_request_time, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(yklogger, getRequestId, arginfo_get_request_id, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_MALIAS(yklogger, get_request_id, getRequestId, arginfo_get_request_id, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_FE_END
};

static char * get_request_server(char *name, uint len){
    zval *args = NULL;

    if ((Z_TYPE(PG(http_globals)[TRACK_VARS_SERVER]) == IS_ARRAY || zend_is_auto_global_str(ZEND_STRL("_SERVER"))) && ((args = zend_hash_str_find_ind(Z_ARRVAL_P(&PG(http_globals)[TRACK_VARS_SERVER]), name, len)) != NULL || (args = zend_hash_str_find_ind(&EG(symbol_table), name, len)) != NULL)) {
        if (!args) {
            return NULL;
        }
        return estrndup(Z_STRVAL_P(args), Z_STRLEN_P(args));
    }
    return NULL;
}

static char * yklogger_array_to_string(const char **file, uint *line, zval *params){
    HashTable *pmht;
    char *buf = NULL;
    size_t pos = 0, buf_length = MAX_PARAMS_STR_LEN;

    buf = emalloc(buf_length + 1);
    pmht = Z_ARRVAL_P(params);
    for (zend_hash_internal_pointer_reset(pmht); zend_hash_has_more_elements(pmht) == SUCCESS; zend_hash_move_forward(pmht)) {
        zend_string *key=NULL;
        char        *tmp=NULL;
        zend_ulong   idx;
        ulong        tmp_len;
        int          type;
        zval        *val;

        type = zend_hash_get_current_key(pmht, &key, &idx);

        if (type == HASH_KEY_NON_EXISTENT) {
            continue;
        }

        if ((val = zend_hash_get_current_data(pmht)) == NULL) {
            continue;
        }

        if (type == HASH_KEY_IS_LONG) {
            convert_to_string(val);
            //php_printf("%ld[%s] \n", idx, Z_STRVAL(tmpcopy));
            tmp_len = spprintf(&tmp, 0, "%ld[%s] ", idx, Z_STRVAL_P(val));
        } else {
            char *source, *target;
            char *end;
            char c;
            zend_string *new_str = NULL;
            size_t newlen;

            if (strcmp(ZSTR_VAL(key), "CUSTOM_LINE") == 0) {
                convert_to_long(val);
                *line = Z_LVAL_P(val);
                continue;
            }

            if (Z_TYPE_P(val) == IS_ARRAY) {
                smart_str buf = {0};

                JSON_G(error_code) = PHP_JSON_ERROR_NONE;
                JSON_G(encode_max_depth) = PHP_JSON_PARSER_DEFAULT_DEPTH;
                php_json_encode(&buf, val, PHP_JSON_UNESCAPED_UNICODE);

                zval_ptr_dtor(val);
                if (JSON_G(error_code) != PHP_JSON_ERROR_NONE) {
                    php_error_docref(NULL, E_NOTICE, "json encode fail: %d", JSON_G(error_code)); \
                        ZVAL_BOOL(val, 0);
                } else {
                    smart_str_0(&buf); /* copy? */
                    ZVAL_STR_COPY(val, buf.s);
                }
                smart_str_free(&buf);
            }

            convert_to_string(val);
            if (strcmp(ZSTR_VAL(key), "CUSTOM_FILE") == 0) {
                *file = estrndup(Z_STRVAL_P(val), Z_STRLEN_P(val));
                continue;
            }

            new_str = zend_string_safe_alloc(4, Z_STRLEN_P(val), 0, 0);
            for(source = (char *) Z_STRVAL_P(val), end=source + Z_STRLEN_P(val), target = ZSTR_VAL(new_str); source < end; source++) {
                c = *source;
                /*php_printf("%c=%d\n", c, (unsigned char)c);*/
                if ((unsigned char) c < 32 || (unsigned char) c > 126) {
                    *target++ = '\\';
                    switch (c) {
                        case '\n': *target++ = 'n'; break;
                        case '\t': *target++ = 't'; break;
                        case '\r': *target++ = 'r'; break;
                        case '\a': *target++ = 'a'; break;
                        case '\v': *target++ = 'v'; break;
                        case '\b': *target++ = 'b'; break;
                        case '\f': *target++ = 'f'; break;
                        default :  *target++ = c;
                    }
                    continue;
                }
                *target++ = c;
            }
            *target = 0;
            newlen = target - ZSTR_VAL(new_str);
            if (newlen < Z_STRLEN_P(val) * 4) {
                new_str = zend_string_truncate(new_str, newlen, 0);
            }

            /*php_printf("%s--%d", ZSTR_VAL(new_str), newlen);*/
            //php_printf("%s[%s] \n", idx, Z_STRVAL(tmpcopy));
            tmp_len = spprintf(&tmp, 0, "%s[%s] ", ZSTR_VAL(key), ZSTR_VAL(new_str));

            zend_string_release(new_str);
        }

        if (pos + tmp_len + 1 >= buf_length) {
            buf_length = buf_length + tmp_len + MAX_PARAMS_STR_LEN;
            buf = erealloc(buf, buf_length);
        }

        memcpy(buf + pos, tmp, tmp_len);
        efree(tmp);

        pos += tmp_len;
    }

    *(buf + pos) = '\0';

    return buf;
}

#define FLUSH_LOGGER_POOL(logfile, file_name_pre, file_name, pool_name) do{\
    php_stream *stream; \
    zval       *logs_pool;\
    char      *file_log_name; \
    \
    logs_pool = zend_read_static_property(yklogger_ce, ZEND_STRL(pool_name), 1); \
    if (zend_hash_num_elements(Z_ARRVAL_P(logs_pool)) < 1) {\
        break; \
    }\
    file_log_name = (char*) emalloc(MAXPATHLEN); \
    sprintf(file_log_name, "%s.%s%s", Z_STRVAL_P(logfile), file_name_pre, ZSTR_VAL(file_name)); \
    \
    stream = php_stream_open_wrapper(file_log_name, "ab", USE_PATH | REPORT_ERRORS, NULL); \
    if (stream == NULL) { \
        php_error_docref(NULL, E_ERROR, "open log file fail, %s", file_log_name); \
    }else{ \
        HashTable *ht; \
        ht = Z_ARRVAL_P(logs_pool); \
        for (zend_hash_internal_pointer_reset(ht); zend_hash_has_more_elements(ht) == SUCCESS; zend_hash_move_forward(ht)) { \
            zval *row; \
            row = zend_hash_get_current_data(ht); \
            php_stream_write(stream, Z_STRVAL_P(row), Z_STRLEN_P(row)); \
        } \
        zend_hash_clean(ht); \
    } \
    php_stream_close(stream); \
    efree(file_log_name); \
}while(0)\

static void yklogger_flush_logs_pool(uint type){
    zval        *logfile;
    zend_string *file_name;

    logfile = zend_read_static_property(yklogger_ce, ZEND_STRL(YKLOGGER_LOGFILE_NAME), 0);

    if (!logfile) {
        php_error_docref(NULL, E_WARNING, "yklogger undeclared log file");
        return;
    }

    file_name = php_format_date("YmdH", 4, time(NULL), 1);

    if (type & LOGS_POOL_TYPE) {
        FLUSH_LOGGER_POOL(logfile, "", file_name, YKLOGGER_POOL_LOG);
    }

    if (type & LOGS_POOL_WF_TYPE) {
        FLUSH_LOGGER_POOL(logfile, YKLOGGER_GE_WARN_PRE_NAME, file_name, YKLOGGER_POOL_WF_LOG);
    }

    zend_string_release(file_name);

    return;
}

static void yklogger_write(uint level_num, const char *level, zend_string *message, zend_long loger_errno, zval *params){
    const char *code_filename = NULL;
    uint        code_line = 0;
    pid_t       php_pid;
    char       *req_ip = NULL, *req_uri = NULL, *req_refer = NULL;
    zend_string *req_date = NULL;
    zval       *request_id;
    zval       *request_start_time;
    char       *params2str = NULL;
    struct      timeval tp = {0};
    double      cost_usec = 0;
    char       *log_row = NULL;
    size_t      log_row_len = 0;
    zval        pool_log_row;
    zval       *pool;
    uint        flush_type;

    req_uri = get_request_server(ZEND_STRS("REQUEST_URI"));
    req_ip = get_request_server(ZEND_STRS("REMOTE_ADDR"));
    req_refer = get_request_server(ZEND_STRS("HTTP_REFERER"));
    req_date = php_format_date("Y-m-d H:i:s", 11, time(NULL), 1);

    request_id = zend_read_static_property(yklogger_ce, ZEND_STRL(YKLOGGER_REQUEST_ID), 1);
    request_start_time = zend_read_static_property(yklogger_ce, ZEND_STRL(YKLOGGER_REQUEST_START_TIME), 1);

    gettimeofday(&tp, NULL);
	/*php_printf("%d", Z_LVAL_P(request_start_time));*/
    cost_usec = tp.tv_sec + tp.tv_usec / MICRO_IN_SEC - Z_LVAL_P(request_start_time);

    params2str = params ? yklogger_array_to_string(&code_filename, &code_line, params) : NULL;

    if (!code_filename || code_line == 0) {
        code_filename = zend_get_executed_filename(TSRMLS_C);
        code_line = zend_get_executed_lineno(TSRMLS_C);
    }
    php_pid = getpid();

    log_row_len = spprintf(&log_row, 0, "%-5s %s,%d [%s:%d] pid[%d] reqip[%s] uri[%s] refer[%s] cost[%f] errno[%d] reqid[%s] %s%s\n", level, ZSTR_VAL(req_date), tp.tv_usec, code_filename, code_line, php_pid, req_ip ? req_ip : "", req_uri ? req_uri : "", req_refer ? req_refer : "", cost_usec, loger_errno, Z_STRVAL_P(request_id), params2str ? params2str : "", ZSTR_VAL(message));

    //php_printf("%s", log_row);

    if (NULL != req_uri) {
        efree(req_uri);
    }
    if (NULL != req_ip) {
        efree(req_ip);
    }
    if (NULL != req_refer) {
        efree(req_refer);
    }
    zend_string_release(req_date);
    if (NULL != params2str) {
        efree(params2str);
    }

    ZVAL_STRINGL(&pool_log_row, log_row, log_row_len);

    if (level_num >= YKLOGGER_LEVEL_WARN) {
        pool = zend_read_static_property(yklogger_ce, ZEND_STRL(YKLOGGER_POOL_WF_LOG), 1);
        flush_type = LOGS_POOL_WF_TYPE;
    }else{
        pool = zend_read_static_property(yklogger_ce, ZEND_STRL(YKLOGGER_POOL_LOG), 1);
        flush_type = LOGS_POOL_TYPE;
    }

    //php_printf("%s", Z_STRVAL(pool_log_row));
    Z_ADDREF(pool_log_row);
    zend_hash_next_index_insert(Z_ARRVAL_P(pool), &pool_log_row);

    if (zend_hash_num_elements(Z_ARRVAL_P(pool)) >= YKLOGGER_G(auto_flush_num)) {
        yklogger_flush_logs_pool(flush_type);
    }

    zval_ptr_dtor(&pool_log_row);
    efree(log_row);

    return;
}

PHP_METHOD(yklogger, init){
    zval *conf;
    zval *logfile, *loglevel;
    uint level_val;
    zend_string *logfile_key, *loglevel_key;
    zval *is_inited = NULL;
    zval log_pool, log_wf_pool;

    is_inited = zend_read_static_property(yklogger_ce, ZEND_STRL(YKLOGGER_LOG_INITED), 1);
    if (Z_LVAL_P(is_inited)) {
        php_error_docref(NULL, E_NOTICE, "yklogger is all ready inited");
        RETURN_NULL();
    }

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "a", &conf) == FAILURE) {
        RETURN_FALSE;
    }

    logfile_key = zend_string_init(ZEND_STRL(YKLOGGER_LOGFILE_NAME), 0);
    logfile = zend_hash_find(Z_ARRVAL_P(conf), logfile_key);
    if (logfile == NULL) {
        zend_string_release(logfile_key);
        php_error_docref(NULL, E_ERROR, "yklogger init undeclared %s", YKLOGGER_LOGFILE_NAME);
        RETURN_FALSE;
    }

    loglevel_key = zend_string_init(ZEND_STRL(YKLOGGER_LOGLEVEL_NAME), 0);
    loglevel = zend_hash_find(Z_ARRVAL_P(conf), loglevel_key);
    if (loglevel == NULL) {
        zend_string_release(logfile_key);
        zend_string_release(loglevel_key);
        php_error_docref(NULL, E_ERROR, "yklogger init undeclared %s", YKLOGGER_LOGLEVEL_NAME);
        RETURN_FALSE;
    }

    if (Z_TYPE_INFO_P(loglevel) == IS_STRING) {
        char *level_str;
        level_str = estrndup(Z_STRVAL_P(loglevel), Z_STRLEN_P(loglevel));
        php_strtoupper(level_str, Z_STRLEN_P(loglevel));
        if (strcmp(level_str, YKLOGGER_LEVEL_DEBUG_NAME) == 0) {
            level_val = YKLOGGER_LEVEL_DEBUG;
        }else if (strcmp(level_str, YKLOGGER_LEVEL_INFO_NAME) == 0 || strcmp(level_str, YKLOGGER_LEVEL_TRACE_NAME) == 0) {
            level_val = YKLOGGER_LEVEL_INFO;
        }else if (strcmp(level_str, YKLOGGER_LEVEL_WARN_NAME) == 0) {
            level_val = YKLOGGER_LEVEL_WARN;
        }else if (strcmp(level_str, YKLOGGER_LEVEL_ERROR_NAME) == 0) {
            level_val = YKLOGGER_LEVEL_ERROR;
        }else if (strcmp(level_str, YKLOGGER_LEVEL_FATAL_NAME) == 0) {
            level_val = YKLOGGER_LEVEL_FATAL;
        }else{
			level_val = 0;
            php_error_docref(NULL, E_ERROR, "yklogger init invalid logLevel name %s", level_str);
        }
        efree(level_str);
    }else{
        convert_to_long_ex(loglevel);
        level_val = Z_LVAL_P(loglevel);
    }

    zend_update_static_property(yklogger_ce, ZEND_STRL(YKLOGGER_LOGFILE_NAME), logfile);
    zend_update_static_property_long(yklogger_ce, ZEND_STRL(YKLOGGER_LOGLEVEL_NAME), level_val);

    ZVAL_LONG(is_inited, 1);
    zend_update_static_property(yklogger_ce, ZEND_STRL(YKLOGGER_LOG_INITED), is_inited);

    array_init_size(&log_pool, 30);
    array_init_size(&log_wf_pool, 10);

    zend_update_static_property(yklogger_ce, ZEND_STRL(YKLOGGER_POOL_LOG), &log_pool);
    zend_update_static_property(yklogger_ce, ZEND_STRL(YKLOGGER_POOL_WF_LOG), &log_wf_pool);

    zend_string_release(logfile_key);
    zend_string_release(loglevel_key);

    RETURN_TRUE;
}

#define CHECK_LOGER_LEVEL(level) \
do{ \
    zval *loger_level; \
    zval *is_inited; \
    \
    is_inited = zend_read_static_property(yklogger_ce, ZEND_STRL(YKLOGGER_LOG_INITED), 1); \
    if (!Z_LVAL_P(is_inited)) { \
        php_error_docref(NULL, E_ERROR, "yklogger not call init function"); \
        RETURN_FALSE; \
    }   \
    \
    loger_level = zend_read_static_property(yklogger_ce, ZEND_STRL(YKLOGGER_LOGLEVEL_NAME), 1);\
    if (level < Z_LVAL_P(loger_level)) { \
        RETURN_NULL(); \
    } \
}while(0);\

#define LOGER_METHODS(level, level_name) {   \
    zend_string *msg;   \
    zend_long  loger_errno = 0; \
    zval *params = NULL; \
    \
    CHECK_LOGER_LEVEL(level); \
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "S|la", &msg, &loger_errno, &params) == FAILURE) { \
        return; \
    } \
    yklogger_write(level, level_name, msg, loger_errno, params); \
    RETURN_TRUE; \
}\


PHP_METHOD(yklogger, debug){

    LOGER_METHODS(YKLOGGER_LEVEL_DEBUG, YKLOGGER_LEVEL_DEBUG_NAME);
}

PHP_METHOD(yklogger, info){

    LOGER_METHODS(YKLOGGER_LEVEL_INFO, YKLOGGER_LEVEL_INFO_NAME);
}

PHP_METHOD(yklogger, warn){

    LOGER_METHODS(YKLOGGER_LEVEL_WARN, YKLOGGER_LEVEL_WARN_NAME);
}

PHP_METHOD(yklogger, error){

    LOGER_METHODS(YKLOGGER_LEVEL_ERROR, YKLOGGER_LEVEL_ERROR_NAME);
}

PHP_METHOD(yklogger, fatal){

    LOGER_METHODS(YKLOGGER_LEVEL_FATAL, YKLOGGER_LEVEL_FATAL_NAME);
}

PHP_METHOD(yklogger, resetRequestTime){
    double request_time = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "|l", &request_time) == FAILURE) {
        return;
    }
    if (!request_time) {
        struct timeval tp = {0};

        gettimeofday(&tp, NULL);
        request_time = tp.tv_sec + tp.tv_usec / MICRO_IN_SEC;
    }

    zend_update_static_property_long(yklogger_ce, ZEND_STRL(YKLOGGER_REQUEST_START_TIME), request_time);
    RETURN_TRUE;
}

PHP_METHOD(yklogger, getRequestId){
    zval *ret;
    ret = zend_read_static_property(yklogger_ce, ZEND_STRL(YKLOGGER_REQUEST_ID), 0);
    ZVAL_COPY_VALUE(return_value, ret);
    return;
}

/* {{{ php_yklogger_init_globals
 */
static void php_yklogger_init_globals(zend_yklogger_globals *yklogger_globals)
{
    yklogger_globals->auto_flush_num = 50;
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(yklogger)
{
    REGISTER_LONG_CONSTANT("YKLOGGER_LEVEL_DEBUG",   YKLOGGER_LEVEL_DEBUG,    CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("YKLOGGER_LEVEL_INFO",    YKLOGGER_LEVEL_INFO,     CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("YKLOGGER_LEVEL_TRACE",   YKLOGGER_LEVEL_TRACE,    CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("YKLOGGER_LEVEL_WARN",    YKLOGGER_LEVEL_WARN,     CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("YKLOGGER_LEVEL_ERROR",   YKLOGGER_LEVEL_ERROR,    CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("YKLOGGER_LEVEL_FATAL",   YKLOGGER_LEVEL_FATAL,    CONST_CS | CONST_PERSISTENT);

    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, "YkLogger", yklogger_methods);
    yklogger_ce = zend_register_internal_class(&ce);

    zend_declare_property_null(yklogger_ce, ZEND_STRL(YKLOGGER_LOG_INITED), ZEND_ACC_STATIC | ZEND_ACC_PRIVATE);
    zend_declare_property_null(yklogger_ce, ZEND_STRL(YKLOGGER_LOGFILE_NAME), ZEND_ACC_STATIC | ZEND_ACC_PRIVATE);
    zend_declare_property_null(yklogger_ce, ZEND_STRL(YKLOGGER_LOGLEVEL_NAME), ZEND_ACC_STATIC | ZEND_ACC_PRIVATE);
    zend_declare_property_null(yklogger_ce, ZEND_STRL(YKLOGGER_REQUEST_ID), ZEND_ACC_STATIC | ZEND_ACC_PRIVATE);
    zend_declare_property_null(yklogger_ce, ZEND_STRL(YKLOGGER_REQUEST_START_TIME), ZEND_ACC_STATIC | ZEND_ACC_PRIVATE);
    zend_declare_property_null(yklogger_ce, ZEND_STRL(YKLOGGER_POOL_LOG), ZEND_ACC_STATIC | ZEND_ACC_PRIVATE);
    zend_declare_property_null(yklogger_ce, ZEND_STRL(YKLOGGER_POOL_WF_LOG), ZEND_ACC_STATIC | ZEND_ACC_PRIVATE);

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(yklogger)
{
    UNREGISTER_INI_ENTRIES();

    return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(yklogger)
{

#if defined(COMPILE_DL_YKLOGGER) && defined(ZTS)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif

    zval request_id;
    zval request_time;
    char request_id_str[33];
    struct timeval tp = {0};
    uint request_start_time;
    PHP_MD5_CTX context;
    unsigned char digest[16];
    char *arg;
    uint arg_len;

    gettimeofday(&tp, NULL);
    request_start_time = tp.tv_sec + tp.tv_usec / MICRO_IN_SEC;

    request_id_str[0] = '\0';
    arg_len = spprintf(&arg, 0, "%s%08x%05x%.8F", "YkLogger", (int)tp.tv_sec, (int)(tp.tv_usec % 0x100000), php_combined_lcg() * 10);

    PHP_MD5Init(&context);
    PHP_MD5Update(&context, arg, arg_len);
    PHP_MD5Final(digest, &context);
    make_digest_ex(request_id_str, digest, 16);

    ZVAL_STRING(&request_id, request_id_str);
    ZVAL_LONG(&request_time, request_start_time);

    zend_update_static_property(yklogger_ce, ZEND_STRL(YKLOGGER_REQUEST_ID), &request_id);
    zend_update_static_property(yklogger_ce, ZEND_STRL(YKLOGGER_REQUEST_START_TIME), &request_time);
    zend_update_static_property_long(yklogger_ce, ZEND_STRL(YKLOGGER_LOG_INITED), 0);

    efree(arg);
    zval_ptr_dtor(&request_id);

	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(yklogger)
{
    zval *is_inited;

    is_inited = zend_read_static_property(yklogger_ce, ZEND_STRL(YKLOGGER_LOG_INITED), 1);

    if (Z_LVAL_P(is_inited)) {
        zval *log_pool, *log_wf_pool;

        yklogger_flush_logs_pool(LOGS_POOL_TYPE | LOGS_POOL_WF_TYPE);

        log_pool = zend_read_static_property(yklogger_ce, ZEND_STRL(YKLOGGER_POOL_LOG), 0);
        zval_ptr_dtor(log_pool);
        log_wf_pool = zend_read_static_property(yklogger_ce, ZEND_STRL(YKLOGGER_POOL_WF_LOG), 0);
        zval_ptr_dtor(log_wf_pool);
    }

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(yklogger)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "Yklogger support", "enabled");
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES();

}
/* }}} */

/* {{{ yklogger_functions[]
 *
 * Every user visible function must have an entry in yklogger_functions[].
 */
const zend_function_entry yklogger_functions[] = {
	PHP_FE_END	/* Must be the last line in yklogger_functions[] */
};
/* }}} */

/* {{{ yklogger_module_entry
 */
zend_module_entry yklogger_module_entry = {
	STANDARD_MODULE_HEADER,
	"YkLogger",
	yklogger_functions,
	PHP_MINIT(yklogger),
	PHP_MSHUTDOWN(yklogger),
	PHP_RINIT(yklogger),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(yklogger),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(yklogger),
	PHP_YKLOGGER_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_YKLOGGER
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE();
#endif
ZEND_GET_MODULE(yklogger)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
