--TEST--
Check Taint with eval
--SKIPIF--
<?php if (!extension_loaded("csas")) print "skip"; ?>
--INI--
csas.enable=1
--FILE--
<?php 
$a = "csased string" . ".";
csas($a); //must use concat to make the string not a internal string(introduced in 5.4)

eval('$b = $a;');
echo $b;
?>
--EXPECTF--
Warning: main(): Attempt to echo a string that might be csased in %s004.php on line %d
csased string.
