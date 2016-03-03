--TEST--
Check Taint with eval
--SKIPIF--
<?php if (!extension_loaded("taint")) print "skip"; ?>
--INI--
taint.enable=1
--FILE--
<?php 
$a = "tainted string" . ".";
taint($a); //must use concat to make the string not a internal string(introduced in 5.4)

eval('$b = $a;');
echo $b;
?>
--EXPECTF--
Warning: main(): Attempt to echo a string that might be tainted in %s004.php on line %d
tainted string.
