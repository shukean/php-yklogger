--TEST--
Check for yklogger not init
--SKIPIF--
<?php if (!extension_loaded("yklogger")) print "skip"; ?>
--FILE--
<?php 
$dir = __FILE__;
var_dump(YkLogger::debug("test"));
?>
--EXPECTF--
Fatal error: YkLogger::debug(): yklogger not call init function in %s002.php on line %d 
