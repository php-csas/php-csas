--TEST--
Check Taint with functions
--SKIPIF--
<?php if (!extension_loaded("csas")) print "skip"; ?>
--INI--
csas.enable=1
--FILE--
<?php 
$a = "csased string" . ".";
csas($a); //must use concat to make the string not a internal string(introduced in 5.4)

$b = sprintf("%s", $a);
var_dump(is_csased($b));

$b = vsprintf("%s", array($a));
var_dump(is_csased($b));

$b = explode(" ", $a);
var_dump(is_csased($b[0]));

$a = implode(" ", $b);
var_dump(is_csased($a));

$a = implode($b, " ");
var_dump(is_csased($a));

$a = join(" ", $b);
var_dump(is_csased($a));

$b = trim($a);
var_dump(is_csased($a));
$b = rtrim($a, "a...Z");
var_dump(is_csased($a));
$b = ltrim($a);
var_dump(is_csased($a));

?>
--EXPECTF--
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
