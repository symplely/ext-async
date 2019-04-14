--TEST--
Process can inherit STDIN from parent.
--SKIPIF--
<?php
if (!extension_loaded('task')) echo 'Test requires the task extension to be loaded';
?>
--STDIN--
Hello World
This is some dummy data
--FILE--
<?php

namespace Concurrent\Process;

$builder = new ProcessBuilder(PHP_BINARY, __DIR__ . '/assets/stderr-output.php');
$builder = $builder->withoutStdout();
$builder = $builder->withStderrInherited(ProcessBuilder::STDOUT);

var_dump('SPAWN');

$builder->execute();

--EXPECT--
string(5) "SPAWN"
HelloWorld :)
