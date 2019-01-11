--TEST--
Check for yklogger init repeat
--SKIPIF--
<?php if (!extension_loaded("yklogger")) print "skip"; ?>
--FILE--
<?php 
YkLogger::init([
    'logFile' => dirname(__LINE__),
    'logLevel' => YKLOGGER_LEVEL_WARN 
]);
YkLogger::init([
    'logFile' => dirname(__LINE__),
    'logLevel' => YKLOGGER_LEVEL_WARN 
]);

?>
--EXPECTF--
Notice: YkLogger::init(): yklogger is all ready inited in %s004.php on line %d 
