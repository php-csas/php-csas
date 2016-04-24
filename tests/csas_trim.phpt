--TEST--
csas_trim() function - basic test for csas_trim()
--FILE--
<?php
  $text   = "\t\tThese are a few words :) ...  ";
  $binary = "\x09Example string\x0A";
  $hello  = "Hello World";
  var_dump($text, $binary, $hello);

  print "\n";
  $trimmed = trim($text);
  var_dump($trimmed);

  $trimmed = trim($text, " \t.");
  var_dump($trimmed);

  $text = $trimmed;
  $trimmed = trim($hello, "Hdle");
  var_dump($trimmed);

  $trimmed = trim($hello, 'HdWr');
  var_dump($trimmed);

  // trim the ASCII control characters at the beginning and end of $binary
  // (from 0 to 31 inclusive)
  $clean = trim($binary, "\x00..\x1F");
  var_dump($clean);
?>
--EXPECT--
string(32) "		These are a few words :) ...  "
string(16) "	Example string
"
string(11) "Hello World"

string(28) "These are a few words :) ..."
string(24) "These are a few words :)"
string(5) "o Wor"
string(9) "ello Worl"
string(14) "Example string"