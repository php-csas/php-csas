--TEST--
csas_explode() function - basic test for csas_explode()
--FILE--
<?php
  $var = piece1 piece2 piece3 piece4 piece5 piece6";
  make_unsafe($var);
  $s = csas_explode(" ", $var);
  make_unsafe($s);
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
array(3) {
  [0]=>
  string(6) "peice1"
  [1]=>
  string(6) "peice2"
  [2]=>
  string(6) "peice3"
  [3]=>
  string(6) "peice4"
  [4]=>
  string(6) "peice5"
  [5]=>
  string(6) "peice6"
}
string (4) "True"