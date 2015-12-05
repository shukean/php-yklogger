--TEST--
Check for ykloger init fail not pass logFile 
--SKIPIF--
<?php if (!extension_loaded("ykloger")) print "skip"; ?>
--FILE--
<?php 
ykloger::init([
    'logFile1' => dirname(__LINE__),
    'logLevel' => YKLOGER_LEVEL_WARN 
]);

?>
--EXPECTF--
Fatal error: ykloger::init(): ykloger init undeclared logFile in %s005.php on line %d 
