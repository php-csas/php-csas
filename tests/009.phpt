--TEST--
Fixed bug that csased info lost if a string is parsed by htmlspecialchars
--SKIPIF--
<?php if (!extension_loaded("csas")) print "skip"; ?>
--INI--
csas.enable=1
--FILE--
<?php
$a = "csased string" . ".";
csas($a); //must use concat to make the string not a internal string(introduced in 5.4)

$b = htmlspecialchars($a);
var_dump(is_csased($b));
var_dump(is_csased($a));
?>
--EXPECTF--
bool(true)
bool(true)
