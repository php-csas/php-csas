--TEST--
csas_substr() function - basic test for csas_substr()
--FILE--

<?php
$rest = substr("abcdef", -1);    // returns "f"
var_dump($rest);
$rest = substr("abcdef", -2);    // returns "ef"
var_dump($rest);
$rest = substr("abcdef", -3, 1); // returns "d"
var_dump($rest);

$var = substr("abcdef", 0, -1);  // returns "abcde"
var_dump($var);
$var = substr("abcdef", 2, -1);  // returns "cde"
var_dump($var);
$var = substr("abcdef", 4, -4);  // returns false
var_dump($var);
$var = substr("abcdef", -3, -1); // returns "de"
var_dump($var);
?>



--EXPECT--
string(1) "f"
string(2) "ef"
string(1) "d"
string(5) "abcde"
string(3) "cde"
bool(false)
string(2) "de"