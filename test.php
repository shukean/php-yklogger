<?php

date_default_timezone_set('Etc/GMT-8');

print_r((new ReflectionClass('YkLogger'))->getStaticProperties());

print_r((new ReflectionClass('YkLogger'))->getMethods());


#var_dump(YKLOGER_LEVEL_DEBUG);

$ret = YkLogger::init([
    'logFile' => './api',
    'logLevel' => YKLOGGER_LEVEL_DEBUG
]);

$ret = YkLogger::debug('xxxxx', 0, ['a' => 'debug', 55 => 'absss']);
$ret = YkLogger::info('aatest', 0, ['a' => 'info']);
$ret = YkLogger::warn('aatest', 0, ['a' => 'warn']);
$ret = YkLogger::error('aatest', 0, ['a' => 'error']);
$ret = YkLogger::fatal('aatest', 0, ['a' => 'fatal']);
