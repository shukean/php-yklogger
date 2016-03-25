<?php
define('YKLOGER_LEVEL_DEBUG', 1);
define('YKLOGER_LEVEL_INFO', 2);
define('YKLOGER_LEVEL_TRACE', 2);
define('YKLOGER_LEVEL_WARN', 4);
define('YKLOGER_LEVEL_ERROR', 8);
define('YKLOGER_LEVEL_FATAL', 16);
class ykloger {
	/* properties */
	static private $_inited = NULL;
	static private $logFile = NULL;
	static private $logLevel = NULL;
	static private $request_id = NULL;
	static private $request_start_time = NULL;
	static private $_pool = NULL;
	static private $_wf_pool = NULL;

	/* methods */
	public static function init(array $config) {
	}
	public static function debug($message, $errno = NULL, array $params = NULL) {
	}
	public static function info($message, $errno = NULL, array $params = NULL) {
	}
	public static function trace($message, $errno = NULL, array $params = NULL) {
	}
	public static function warn($message, $errno = NULL, array $params = NULL) {
	}
	public static function error($message, $errno = NULL, array $params = NULL) {
	}
	public static function fatal($message, $errno = NULL, array $params = NULL) {
	}
	public static function resetRequestTime($time = NULL) {
	}
	public static function reset_request_time($time = NULL) {
	}
	public static function getRequestId() {
	}
	public static function get_request_id() {
	}
}
class bqlogger {
	/* properties */
	static private $_inited = NULL;
	static private $logFile = NULL;
	static private $logLevel = NULL;
	static private $request_id = NULL;
	static private $request_start_time = NULL;
	static private $_pool = NULL;
	static private $_wf_pool = NULL;

	/* methods */
	public static function init(array $config) {
	}
	public static function debug($message, $errno = NULL, array $params = NULL) {
	}
	public static function info($message, $errno = NULL, array $params = NULL) {
	}
	public static function trace($message, $errno = NULL, array $params = NULL) {
	}
	public static function warn($message, $errno = NULL, array $params = NULL) {
	}
	public static function error($message, $errno = NULL, array $params = NULL) {
	}
	public static function fatal($message, $errno = NULL, array $params = NULL) {
	}
	public static function resetRequestTime($time = NULL) {
	}
	public static function reset_request_time($time = NULL) {
	}
	public static function getRequestId() {
	}
	public static function get_request_id() {
	}
}
