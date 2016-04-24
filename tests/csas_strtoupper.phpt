--TEST--
csas_strtoupper() function - basic test for csas_strtoupper()
--FILE--

<?php
$str = "Mary Had A Little Lamb and She LOVED It So";
$str = strtoupper($str);
var_dump($str); // Prints mary had a little lamb and she loved it so
?>

--EXPECT--
string(42) "MARY HAD A LITTLE LAMB AND SHE LOVED IT SO"