--TEST--
csas_str_replace() function - basic test for csas_str_replace()
--FILE--


<?php
// Provides: <body text='black'>
$bodytag = "%body%";
make_unsafe($bodytag);
$bodytag = str_replace($bodytag, "black", "<body text='%body%'>");

// Provides: Hll Wrld f PHP
$vowels = array("a", "e", "i", "o", "u", "A", "E", "I", "O", "U");
$onlyconsonants = str_replace($vowels, "", "Hello World of PHP");

// Provides: You should eat pizza, beer, and ice cream every day
$phrase  = "You should eat fruits, vegetables, and fiber every day.";
$healthy = array("fruits", "vegetables", "fiber");
$yummy   = array("pizza", "beer", "ice cream");

$newphrase = str_replace($healthy, $yummy, $phrase);

// Provides: 2
$str = str_replace("ll", "", "good golly miss molly!", $count);
echo $count;
if (csas_get_safety($bodytag)!=0xFFFFFFFF)) var_dump(true);
if (csas_get_safety($newphrase==0xFFFFFFF)) var_dump(true);
?>


--EXPECT--
string(19) "<body text='black'>"
string(14) "Hll Wrld f PHP"
string(51) "You should eat pizza, beer, and ice cream every day"
string(1) "2"
bool(true)
bool(true)