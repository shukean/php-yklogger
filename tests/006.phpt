--TEST--
Check for yklogger init fail not pass logLevel 
--SKIPIF--
<?php if (!extension_loaded("yklogger")) print "skip"; ?>
--FILE--
<?php 
YkLogger::init([
    'logFile' => dirname(__LINE__),
    'logLevel2' => YKLOGGER_LEVEL_WARN 
]);

?>
--EXPECTF--
Fatal error: YkLogger::init(): yklogger init undeclared logLevel in %s006.php on line %d 
