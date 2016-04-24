--TEST--
csas_implode() function - basic test for csas_implode()
--FILE--
<?php
  $var = array('lastname', 'email', "phone");
  $s = implode(",", $var);
  var_dump($s);
?>
--EXPECT--
string(20) "lastname,email,phone"