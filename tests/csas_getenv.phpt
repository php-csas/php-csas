--TEST--
csas_getenv()
--SKIPIF--
<?php if (!extension_loaded("csas")) print "skip"; ?>
--INI--
csas.enable=1
--FILE--

<?php
$testenv = 2;
putenv("TESTENV=$testenv");
$a = getenv("TESTENV");
var_dump($a);
?>

--EXPECTF--
string(1) "2"