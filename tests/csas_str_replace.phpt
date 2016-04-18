--TEST--
csas_str_replace() function - basic test for csas_str_replace()
--SKIPIF--
<?php if (!extension_loaded("csas")) print "skip"; ?>
--INI--
csas.enable=1
report_memleaks=Off
--FILE--
<?php
// Provides: <body text='black'>
$bodytag = str_replace("%body%", "black", "<body text='%body%'>");
echo $bodytag;
echo "\n";

// Provides: Hll Wrld f PHP
$vowels = array("a", "e", "i", "o", "u", "A", "E", "I", "O", "U");
$onlyconsonants = str_replace($vowels, "", "Hello World of PHP");
echo $onlyconsonants;
echo "\n";

// Provides: You should eat pizza, beer, and ice cream every day
$phrase  = "You should eat fruits, vegetables, and fiber every day";
$healthy = array("fruits", "vegetables", "fiber");
$yummy   = array("pizza", "beer", "ice cream");

$newphrase = str_replace($healthy, $yummy, $phrase);
echo $newphrase;
echo "\n";

// Provides: 2
$count = 1;
$str = str_replace("ll", "", "good golly miss molly!", $count);
echo $count;
?>


--EXPECT--
<body text='black'>
Hll Wrld f PHP
You should eat pizza, beer, and ice cream every day
2