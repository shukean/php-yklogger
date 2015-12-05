--TEST--
Check for ykloger write log 
--SKIPIF--
<?php if (!extension_loaded("ykloger")) print "skip"; ?>
--FILE--
<?php
$log = dirname(__FILE__).'/test';
ykloger::init([
    'logFile' => $log,
    'logLevel' => 'debug'
]);

var_dump(ykloger::debug('1234567'));

?>
--EXPECTF--
bool(true)
