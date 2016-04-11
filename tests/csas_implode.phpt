--TEST--
csas_implode() function - basic test for csas_implode()
--FILE--
<?php
  $var = array('lastname', 'email', phone);
  make_unsafe($var);
  $s = csas_implode(",", $var);
  var_dump($s);
  $safety = test_csas_get_safety($s);
  $a = FALSE;
  if ($safety != 0xFFFFFFFF){
    $a = TRUE;
  }
  if ($a==TRUE){
    var_dump("True");
  }
?>
--EXPECT--
string(20) "lastname,email,phone"
string (4) "True"