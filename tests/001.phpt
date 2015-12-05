--TEST--
Check for ykloger presence
--SKIPIF--
<?php if (!extension_loaded("ykloger")) print "skip"; ?>
--FILE--
<?php 
echo "ykloger extension is available\n";

if(defined('YKLOGER_LEVEL_DEBUG')){
    echo "var defined\n";
}

?>
--EXPECT--
ykloger extension is available
var defined
