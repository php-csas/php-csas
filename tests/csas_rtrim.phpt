--TEST--
csas_rtrim() function - basic test for csas_rtrim()
--FILE--

<?php

$text = "\t\tThese are a few words :) ...  ";
$binary = "\x09Example string\x0A";
$hello  = "Hello World";
var_dump($text, $binary, $hello);

print "\n";

$trimmed = rtrim($text);
var_dump($trimmed);

$trimmed = rtrim($text, " \t.");
var_dump($trimmed);

$trimmed = rtrim($hello, "Hdle");
var_dump($trimmed);

// trim the ASCII control characters at the end of $binary
// (from 0 to 31 inclusive)
$clean = rtrim($binary, "\x00..\x1F");
var_dump($clean);
?>

--EXPECT--
string(32) "		These are a few words :) ...  "
string(16) "	Example string
"
string(11) "Hello World"

string(30) "		These are a few words :) ..."
string(26) "		These are a few words :)"
string(9) "Hello Wor"
string(15) "	Example string"