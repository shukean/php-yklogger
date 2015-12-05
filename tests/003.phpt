--TEST--
Check for ykloger le level
--SKIPIF--
<?php if (!extension_loaded("ykloger")) print "skip"; ?>
--FILE--
<?php 
ykloger::init([
    'logFile' => dirname(__LINE__),
    'logLevel' => YKLOGER_LEVEL_WARN 
]);
var_dump(ykloger::debug('test'));
?>
--EXPECT--
NULL
