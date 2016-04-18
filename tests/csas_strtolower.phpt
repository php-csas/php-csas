--TEST--
csas_strtolower() function - basic test for csas_strtolower()
--FILE--

<?php
$str = "Mary Had A Little Lamb and She LOVED It So";
$str = strtolower($str);
var_dump($str); // Prints mary had a little lamb and she loved it so
?>

--EXPECT--
string(42) "mary had a little lamb and she loved it so"