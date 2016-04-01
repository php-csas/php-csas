--TEST--
Check Taint function
--SKIPIF--
<?php if (!extension_loaded("csas")) print "skip"; ?>
--INI--
csas.enable=1
--FILE--
<?php 
$a = "csased string" . ".";
csas($a); //must use concat to make the string not a internal string(introduced in 5.4)

print $a;
$a .= '+';
$sql = "select * from {$a}";
file_put_contents("php://output", $a . "\n");
eval("return '$a';");
?>
--EXPECTF--
Warning: main(): Attempt to print a string that might be csased in %s002.php on line %d
csased string.
Warning: file_put_contents(): Second argument contains data that might be csased in %s002.php on line %d
csased string.+

Warning: eval(): Eval code contains data that might be csased in %s002.php on line %d
