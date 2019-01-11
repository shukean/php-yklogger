--TEST--
Check for yklogger init fail not pass logFile 
--SKIPIF--
<?php if (!extension_loaded("yklogger")) print "skip"; ?>
--FILE--
<?php 
Yklogger::init([
    'logFile1' => dirname(__LINE__),
    'logLevel' => YKLOGGER_LEVEL_WARN 
]);

?>
--EXPECTF--
Fatal error: YkLogger::init(): yklogger init undeclared logFile in %s005.php on line %d 
