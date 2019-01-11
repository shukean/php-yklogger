--TEST--
Check for yklogger init fail logLevel name 
--SKIPIF--
<?php if (!extension_loaded("yklogger")) print "skip"; ?>
--FILE--
<?php 
YkLogger::init([
    'logFile' => dirname(__LINE__),
    'logLevel' => 'warnnnn'
]);

?>
--EXPECTF--
Fatal error: YkLogger::init(): yklogger init invalid logLevel name WARNNNN in %s007.php on line %d 
