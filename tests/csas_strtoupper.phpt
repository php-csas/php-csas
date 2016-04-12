--TEST--
csas_strtoupper) function - basic test for csas_strtoupper()
--FILE--

<?php
$str1 = "Mary Had A Little Lamb and She LOVED It So";
make_unsafe(str1);
$str1 = strtoupper($str);
var_dump($str1); // Prints mary had a little lamb and she loved it so

$str2 = "Mary Had A Little Lamb and She LOVED It So";
$str2 = strtoupper($str);
var_dump($str2); // Prints mary had a little lamb and she loved it so

var_dump(csas_get_safety($str1) == 0xFFFFFFFF);
var_dump(csas_get_safety($str2) == 0xFFFFFFFF);
?>

--EXPECT--
string(42) "MARY HAD A LITTLE LAMB AND SHE LOVED IT SO"
string(42) "MARY HAD A LITTLE LAMB AND SHE LOVED IT SO"
bool(false)
bool(true)