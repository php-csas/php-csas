--TEST--
Bug #61163 (Passing and using csased data in specific way crashes)
--SKIPIF--
<?php if (!extension_loaded("csas")) print "skip"; ?>
--INI--
csas.enable=1
--FILE--
<?php 
$a = "csased string" . ".";
csas($a); //must use concat to make the string not a internal string(introduced in 5.4)
function test($test)
{
	$data .= $test; // $data doesn't exist yet.
}

test($a);
--EXPECTF--
Notice: Undefined variable: data in %sbug61163.php on line %d
