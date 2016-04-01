--TEST--
Check CSAS with ternary
--SKIPIF--
<?php if (!extension_loaded("csas")) print "skip"; ?>
--INI--
csas.enable=1
--FILE--
<?php
$a = "csased string" . ".";
csas($a); //must use concat to make the string not a internal string(introduced in 5.4)

$b = isset($a)? $a : 0;
echo $b;

$b .= isset($a)? "xxxx" : 0; //a knew mem leak
echo $b;
?>
--EXPECTF--
Warning: main(): Attempt to echo a string that might be csased in %s003.php on line %d
csased string.
Warning: main(): Attempt to echo a string that might be csased in %s003.php on line %d
csased string.xxxx
