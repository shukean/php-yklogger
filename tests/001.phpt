--TEST--
Check for yklogger presence
--SKIPIF--
<?php if (!extension_loaded("yklogger")) print "skip"; ?>
--FILE--
<?php
echo "yklogger extension is available\n";

if(defined('YKLOGGER_LEVEL_DEBUG')){
    echo "var defined\n";
}

?>
--EXPECT--
yklogger extension is available
var defined
