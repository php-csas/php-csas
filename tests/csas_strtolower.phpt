--TEST--
csas_strtolower() function - basic test for csas_strtolower()
--FILE--

<?php
$str1 = "Mary Had A Little Lamb and She LOVED It So";
make_unsafe(str1);
$str1 = strtolower($str);
var_dump($str1); // Prints mary had a little lamb and she loved it so

$str2 = "Mary Had A Little Lamb and She LOVED It So";
$str2 = strtolower($str);
var_dump($str2); // Prints mary had a little lamb and she loved it so

var_dump(csas_get_safety($str1) == 0xFFFFFFFF);
var_dump(csas_get_safety($str2) == 0xFFFFFFFF);
?>

--EXPECT--
string(42) "mary had a little lamb and she loved it so"
string(42) "mary had a little lamb and she loved it so"
bool(false)
bool(true)