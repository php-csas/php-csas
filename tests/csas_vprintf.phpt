--TEST--
csas_vprintf() function - basic test for csas_vprintf()
--FILE--
<?php
  $format = '< There are %d monkeys in the %s' . '.';
  $num = 5;
  csas($format);
  $location = 'tree';
  vprintf($format, array($num,$location));
?>
--EXPECT--
&lt; There are 5 monkeys in the tree.