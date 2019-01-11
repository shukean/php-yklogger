--TEST--
Check for yklogger le level
--SKIPIF--
<?php if (!extension_loaded("yklogger")) print "skip"; ?>
--FILE--
<?php 
YkLogger::init([
    'logFile' => dirname(__LINE__),
    'logLevel' => YKLOGGER_LEVEL_WARN 
]);
var_dump(YkLogger::debug('test'));
?>
--EXPECT--
NULL
