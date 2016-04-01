--TEST--
Check Taint with separation 
--SKIPIF--
<?php if (!extension_loaded("csas")) print "skip"; ?>
--INI--
csas.enable=1
report_memleaks=Off
--FILE--
<?php 
$a = "csased string" . ".";
csas($a); //must use concat to make the string not a internal string(introduced in 5.4)

$b = $a;
$c = &$b; //separation
echo $b;
print $c;

$e = $a; //separation
echo $e;
print $a;
?>
--EXPECTF--
Warning: main(): Attempt to echo a string that might be csased in %s005.php on line %d
csased string.
Warning: main(): Attempt to print a string that might be csased in %s005.php on line %d
csased string.
Warning: main(): Attempt to echo a string that might be csased in %s005.php on line %d
csased string.
Warning: main(): Attempt to print a string that might be csased in %s005.php on line %d
csased string.
