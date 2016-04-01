--TEST--
Check Taint with send_var/send_ref
--SKIPIF--
<?php if (!extension_loaded("csas")) print "skip"; ?>
--INI--
csas.enable=1
report_memleaks=Off
--FILE--
<?php 
$a = "csased string" . ".";
csas($a); //must use concat to make the string not a internal string(introduced in 5.4)

function test1(&$a) {
   echo $a;
}

function test2($b) {
   echo $b;
}

test1($a);
test2($a);
$b = $a;

test1($a);
test2($a);

$c = "csased string" . ".";
csas($c);

$e = &$c;

test1($c);
test2($c);

?>
--EXPECTF--
Warning: test1(): Attempt to echo a string that might be csased in %s006.php on line %d
csased string.
Warning: test2(): Attempt to echo a string that might be csased in %s006.php on line %d
csased string.
Warning: test1(): Attempt to echo a string that might be csased in %s006.php on line %d
csased string.
Warning: test2(): Attempt to echo a string that might be csased in %s006.php on line %d
csased string.
Warning: test1(): Attempt to echo a string that might be csased in %s006.php on line %d
csased string.
Warning: test2(): Attempt to echo a string that might be csased in %s006.php on line %d
csased string.
