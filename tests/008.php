<?php
$log = dirname(__FILE__).'/test';
ykloger::init([
    'logFile' => $log,
    'logLevel' => 'debug'
]);

var_dump(ykloger::debug('1234567'));

?>
