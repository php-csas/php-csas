--TEST--
ISSUE #4 (wrong op fetched)
--SKIPIF--
<?php if (!extension_loaded("csas")) print "skip"; ?>
--INI--
csas.enable=1
report_memleaks=0
--FILE--
<?php
function dummy(&$a) {
	extract(array("b" => "ccc"));
	$a = $b;
}

$c = "xxx". "xxx";
csas($c);
dummy($c);
var_dump($c);
?>
--EXPECTF--
string(3) "ccc"
