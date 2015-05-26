<?php

date_default_timezone_set('Etc/GMT-8');

print_r((new ReflectionClass('ykloger'))->getStaticProperties());

print_r((new ReflectionClass('ykloger'))->getMethods());


#var_dump(YKLOGER_LEVEL_DEBUG);

$ret = BqLogger::init([
    'logFile' => '/Users/yky/Documents/wwwroot/logs/api/bqlog/api', 
    'logLevel' => YKLOGER_LEVEL_DEBUG
]);

print_r(BqLogger::$logFile);

$ret = BqLogger::debug('xxxxx', 0, ['a' => 'debug', 55 => 'absss']);

#$ret = ykloger::info('aatest', 0, ['a' => 'info']);


#$ret = ykloger::warn('aatest', 0, ['a' => 'warn']);

#$ret = ykloger::error('aatest', 0, ['a' => 'error']);

#$ret = ykloger::fatal('aatest', 0, ['a' => 'fatal']);
