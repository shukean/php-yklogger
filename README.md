# php-ykloger
php日志记录扩展. 共有5中级别. 分为:  debug,  info(trace), warn, error, fatal.


##init
date_default_timezone_set('Etc/GMT-8');  
ykloger::init([  
    'logFile' => path,   
    'logLevel' => level  
]);  

path 为日志存放目录.  
level 为日志级别. 低于级别的方法调用不会写入日志.  日志级别定义的有常量, 分别为:   
1.YKLOGER_LEVEL_DEBUG  
2.YKLOGER_LEVEL_INFO  
3.YKLOGER_LEVEL_WARN  
4.YKLOGER_LEVEL_ERROR  
5.YKLOGER_LEVEL_FATAL  

##methods
ykloger::debug(string $message, uint $errno = 0, arary $params = []);  
ykloger::info(string $message, uint $errno = 0 , arary $params = []); // ykloger::trace 是info的别名  
ykloger::warn(string $message, uint $errno = 0, arary $params = []);  
ykloger::error(string $message, uint $errno = 0, arary $params = []);   
ykloger::fatal(string $message, uint $errno = 0, arary $params = []);  

##methods argument
$message  日志描述  
$errno    错误码  
$params   需要记录的其他字段信息 (只支持一级数组)  

##other methods
ykloger::reset_request_time(uint timestamp = 0); // ykloger::resetStartTime 是reset_request_time的别名  
重置初始化的时间  

string $request_id = ykloger::get_request_id(); // ykloger::getRequestId 是get_request_id的别名  
获取本次request的随机ID  

##other tips
BqLogger alias ykloger  
BqLogger 是 ykloger 的别名. 即, 使用 ykloger::method 与 BqLogger:method 是相同的.  

##log file
etc: api.2015052314   api.wf.2015052314  
wf 为 warn error  fatal 的日志, 日志按小时划分.  
logFile 最后一个斜线的字符为日志文件的前缀.   


##log content
级别 时间 [文件:行数] [php进程号] reqip[IP] uri[uri] refer[url] cost[耗时] errno[0]  params message  
params 会按照 [key] value 的方式连接.  
