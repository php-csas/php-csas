--TEST--
csas_ltrim() function - basic test for csas_ltrim()
--SKIPIF--
<?php if (!extension_loaded("csas")) print "skip"; ?>
--INI--
csas.enable=1
--FILE--
<?php
$text = "\t\tThese are a few words :) ...  ";
$binary = "\x09Example string\x0A";
$hello  = "Hello World";
var_dump($text, $binary, $hello);

print "\n";

$trimmed = ltrim($text);
var_dump($trimmed);

$trimmed = ltrim($text, "\t");
var_dump($trimmed);

$text = $trimmed;
$trimmed = ltrim($hello, "Hdle");
var_dump($trimmed);

// trim the ASCII control characters at the beginning of $binary
// (from 0 to 31 inclusive)
$clean = ltrim($binary, "\x00..\x1F");
var_dump($clean);
?>

--EXPECT--
string(32) "		These are a few words :) ...  "
string(16) "	Example string
"
string(11) "Hello World"

string(30) "These are a few words :) ...  "
string(30) "These are a few words :) ...  "
string(7) "o World"
string(15) "Example string
"