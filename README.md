# php-ykloger

date_default_timezone_set('Etc/GMT-8');
#var_dump(YKLOGER_LEVEL_DEBUG);
#include "/Users/yky/Documents/wwwroot/new-beijing/application/library/bqlogger/BqLogger.php";
ykloger::init([
    'logFile' => '/Users/yky/Documents/wwwroot/logs/api/bqlog/api', 
    'logLevel' => YKLOGER_LEVEL_DEBUG, //YKLOGER_LEVEL_INFO, YKLOGER_LEVEL_WARN, YKLOGER_LEVEL_ERROR, YKLOGER_LEVEL_FATAL
]);
ykloger::debug($message, $errno, arary $params);
ykloger::info($message, $errno, arary $params); // ykloger::trace($message, $errno, arary $params);;
ykloger::warn($message, $errno, arary $params);
ykloger::error($message, $errno, arary $params);
ykloger::fatal($message, $errno, arary $params);

//重置用户请求的开始时间
ykloger::reset_request_time(timestamp); // ykloger::resetStartTime(timestamp);

//获取一次请求的requestid
ykloger::get_request_id(); // ykloger::getRequestId();

BqLogger alias ykloger;

//log file
api.2015052314   api.wf.2015052314

//log content
级别 时间 [文件:行数] [进行号] reqip[IP] uri[uri] refer[空] cost[耗时] errno[]  params message
