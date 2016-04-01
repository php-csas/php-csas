--TEST--
Bug #63123 (Hash pointer should be reset at the end of function:php_csas_mark_strings)
--SKIPIF--
<?php if (!extension_loaded("csas")) print "skip"; ?>
--INI--
csas.enable=1
report_memleaks=0
--FILE--
<?php 

$str = 'a,' . 'b';
csas($str);
$a = explode(',', $str);
while (list($key, $val) = each($a)) {
    echo $val;
}

?>
--EXPECTF--
Warning: main(): Attempt to echo a string that might be csased in %sbug63123.php on line %d
a
Warning: main(): Attempt to echo a string that might be csased in %Sbug63123.php on line %d
b
