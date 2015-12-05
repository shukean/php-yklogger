--TEST--
Check for ykloger not init
--SKIPIF--
<?php if (!extension_loaded("ykloger")) print "skip"; ?>
--FILE--
<?php 
$dir = __FILE__;
var_dump(ykloger::debug("test"));
?>
--EXPECTF--
Fatal error: ykloger::debug(): ykloger not call init function in %s002.php on line %d 
