--TEST--
csas_sprintf() function - basic test for strtr()
--FILE--
<?php
  $format = 'There are %d monkeys in the %s';
  $num = 5;
  $location = 'tree';
  $s = sprintf($format, $num, $location);
  var_dump($s);
?>
--EXPECT--
string(31) "There are 5 monkeys in the tree"