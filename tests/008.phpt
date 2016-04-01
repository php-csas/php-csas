--TEST--
Check Taint with more functions
--SKIPIF--
<?php if (!extension_loaded("csas")) print "skip"; ?>
--INI--
csas.enable=1
--FILE--
<?php 
$a = "csased string" . ".";
csas($a); //must use concat to make the string not a internal string(introduced in 5.4)

$b = strstr($a, "s");
var_dump(is_csased($b));

$b = substr($a, 0, 4);
var_dump(is_csased($b));

$b = str_replace("str,", "btr", $a);
var_dump(is_csased($b));

$b = str_pad($a, 32);
var_dump(is_csased($b));

$b = str_pad("test", 32, $a);
var_dump(is_csased($b));

$b = strtolower($a);
var_dump(is_csased($b));

$b = strtoupper($a);
var_dump(is_csased($b));
?>
--EXPECTF--
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
