--TEST--
Check for ykloger init fail logLevel name 
--SKIPIF--
<?php if (!extension_loaded("ykloger")) print "skip"; ?>
--FILE--
<?php 
ykloger::init([
    'logFile' => dirname(__LINE__),
    'logLevel' => 'warnnnn'
]);

?>
--EXPECTF--
Fatal error: ykloger::init(): ykloger init invalid logLevel name WARNNNN in %s007.php on line %d 
