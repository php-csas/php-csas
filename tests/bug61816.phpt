--TEST--
Bug #61816 (Segmentation fault)
--SKIPIF--
<?php if (!extension_loaded("csas")) print "skip"; ?>
--INI--
csas.enable=1
--FILE--
<?php
$a = "csased string" . ".\n";
csas($a);
$b = array("");
$b[0] .= $a;
var_dump(is_csased($b[0]));
$c = new stdClass();
$c->foo = "this is";
$c->foo .= $b[0];
echo $b[0];  // Segmentation fault
var_dump(is_csased($c->foo));
?>
--EXPECTF--
bool(true)

Warning: main(): Attempt to echo a string that might be csased in %sbug61816.php on line %d
csased string.
bool(true)
