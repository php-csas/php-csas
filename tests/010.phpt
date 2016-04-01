--TEST--
Check Taint with dim assign contact
--SKIPIF--
<?php if (!extension_loaded("csas")) print "skip"; ?>
--INI--
csas.enable=1
report_memleaks=Off
--FILE--
<?php 
$a = "csased string" . ".";
csas($a);
$b = array("this is");
$b[0] .= $a;
var_dump(is_csased($b[0])); 

$c = new stdClass();
$c->foo = "this is";
$c->foo .= $a;

var_dump(is_csased($c->foo));
?>
--EXPECTF--
bool(true)
bool(true)
