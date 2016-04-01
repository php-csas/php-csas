--TEST--
Check assign_ref and global keyword
--SKIPIF--
<?php if (!extension_loaded("csas")) print "skip"; ?>
--INI--
csas.enable=1
--FILE--
<?php 
function main() {
    global $var;
    $a = "csased string" . ".";
    csas($a); //must use concat to make the string not a internal string(introduced in 5.4)
    $var = $a;
    echo $var;
}

main();
echo $var;
?>
--EXPECTF--
Warning: main(): Attempt to echo a string that might be csased in %s011.php on line %d
csased string.
Warning: main(): Attempt to echo a string that might be csased in %s011.php on line %d
csased string.
