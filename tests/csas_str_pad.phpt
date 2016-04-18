--TEST--
csas_str_pad() function - basic test for csas_str_pad()
--FILE--
<?php
$input = "Alien";
var_dump(str_pad($input, 10));
var_dump(str_pad($input, 10, "-=", STR_PAD_LEFT));
var_dump(str_pad($input, 10, "_", STR_PAD_BOTH));
var_dump(str_pad($input,  6, "___"));
var_dump(str_pad($input,  3, "*"));
?>


--EXPECT--
string(10) "Alien     "
string(10) "-=-=-Alien"
string(10) "__Alien___"
string(6) "Alien_"
string(5) "Alien"