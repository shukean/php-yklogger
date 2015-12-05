--TEST--
Check for ykloger init repeat
--SKIPIF--
<?php if (!extension_loaded("ykloger")) print "skip"; ?>
--FILE--
<?php 
ykloger::init([
    'logFile' => dirname(__LINE__),
    'logLevel' => YKLOGER_LEVEL_WARN 
]);
ykloger::init([
    'logFile' => dirname(__LINE__),
    'logLevel' => YKLOGER_LEVEL_WARN 
]);

?>
--EXPECTF--
Notice: ykloger::init(): ykloger is all ready inited in %s004.php on line %d 
