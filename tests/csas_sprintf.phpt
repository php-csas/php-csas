--TEST--
csas_sprintf() function - basic test for strtr()
--FILE--
<?php
  $format = 'There are %d monkeys in the %s'
  $num = 5;
  $location = 'tree';
  csas_make_unsafe($format);
  $s = sprintf($format, $num, $location);
  var_dump($s);
  $safety = csas_get_safety($s);
  $a = FALSE;
  var_dump($safety != 0xFFFFFFFF);
?>
--EXPECT--
string(31) "There are 5 monkeys in the tree"
bool(true);