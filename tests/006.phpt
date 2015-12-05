--TEST--
Check for ykloger init fail not pass logLevel 
--SKIPIF--
<?php if (!extension_loaded("ykloger")) print "skip"; ?>
--FILE--
<?php 
ykloger::init([
    'logFile' => dirname(__LINE__),
    'logLevel2' => YKLOGER_LEVEL_WARN 
]);

?>
--EXPECTF--
Fatal error: ykloger::init(): ykloger init undeclared logLevel in %s006.php on line %d 
