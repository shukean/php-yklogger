--TEST--
Check for yklogger write log 
--SKIPIF--
<?php if (!extension_loaded("yklogger")) print "skip"; ?>
--FILE--
<?php
$log = dirname(__FILE__).'/test';
YkLogger::init([
    'logFile' => $log,
    'logLevel' => 'debug'
]);

var_dump(YkLogger::debug('1234567'));

?>
--EXPECTF--
bool(true)
