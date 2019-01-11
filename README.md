# php- YkLogger
php日志记录扩展. 共有5种级别. 分为:  debug,  info(trace), warn, error, fatal.  
(php7 版本)

## install
mac or linux:  
1.  wget https://github.com/shukean/php-yklogger.git
2.  unzip master.php  
3.  phpize  
4.  ./configure --with-php-config=your_php_path/php-config  
5.  make && make install  

## php.ini
```
[YkLogger]
ykgogger.auto_flush_num = 50  
```
达到多少条日志时, 才开始写入文件.  
当request 结束时, 也会写入.  

## init
```
date_default_timezone_set('Etc/GMT-8');  
 YkLogger::init([  
    'logFile' => path,  
    'logLevel' => level  
]);  
```
> path 为日志存放目录.  
> level 为日志级别. 低于级别的方法调用不会写入日志.  日志级别定义的有常量, 分别为:  

1.YKLOGGER_LEVEL_DEBUG  
2.YKLOGGER_LEVEL_INFO  
3.YKLOGGER_LEVEL_WARN  
4.YKLOGGER_LEVEL_ERROR  
5.YKLOGGER_LEVEL_FATAL  

## methods
```
YkLogger::debug(string $message, uint $errno = 0, arary $params = []);  
YkLogger::info(string $message, uint $errno = 0 , arary $params = []); //  YkLogger::trace 是info的别名  
YkLogger::warn(string $message, uint $errno = 0, arary $params = []);  
YkLogger::error(string $message, uint $errno = 0, arary $params = []);  
YkLogger::fatal(string $message, uint $errno = 0, arary $params = []);  
```

## methods argument
$message  日志描述  
$errno    错误码  
$params   需要记录的其他字段信息, 二维数组将会转为json存入  

## other methods
 YkLogger::resetRequestTime(uint timestamp = 0); //  YkLogger::reset_request_time 是resetRequestTime的别名  
重置初始化的时间  

string $request_id =  YkLogger::getRequestId(); //  YkLogger::get_request_id 是getRequestId的别名  
获取本次request的随机ID, 不保证不会重复  

## log file
etc: api.2015052314   api.wf.2015052314  
wf 为 warn error  fatal 的日志, 日志按小时划分.  
logFile 最后一个斜线的字符为日志文件的前缀.  


## log content
级别 时间 [文件:行数] [php进程号] reqip[IP] uri[uri] refer[url] cost[耗时] errno[0]  params message  
params 会按照 [key] value 的方式连接.  
