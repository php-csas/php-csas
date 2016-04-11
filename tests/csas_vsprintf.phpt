--TEST--
csas_vsprintf() function - basic test for csas_vsprintf()
--FILE--
<?php
  $format = 'There are %d monkeys in the %s'
  csas_make_unsafe($format);
  $num = 5;
  $location = 'tree';
  $s = vsprintf($format, array('tree',5));
  var_dump($s);
  $safety = test_csas_get_safety($s);
  $a = FALSE;
  var_dump($safety != 0xFFFFFFFF)
?>
--EXPECT--
string(31) "There are 5 monkeys in the tree"
bool(true);