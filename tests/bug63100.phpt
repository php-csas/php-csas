--TEST--
Bug #63100 (array_walk_recursive behaves wrongly when csas enabled)
--SKIPIF--
<?php if (!extension_loaded("csas")) print "skip"; ?>
--INI--
csas.enable=1
report_memleaks=0
--FILE--
<?php 
$a = array();
$a[0] = "csased string" . "<>";
csas($a[0]);

function xxx(&$item) {
    $item = htmlspecialchars($item);
}

array_walk_recursive($a, "xxx");

echo $a[0];

?>
--EXPECTF--
csased string&lt;&gt;
