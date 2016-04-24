--TEST--
csas_vsprintf() function - basic test for csas_vsprintf()
--FILE--
<?php
  $format = 'There are %d monkeys in the %s';
  $num = 5;
  $location = 'tree';
  $s = vsprintf($format, array($num,$location));
  var_dump($s);
?>
--EXPECT--
string(31) "There are 5 monkeys in the tree"