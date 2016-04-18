--TEST--
csas_explode() function - basic test for csas_explode()
--FILE--
<?php
  $var = "piece1 piece2 piece3 piece4 piece5 piece6";
  $s = explode(" ", $var);
  var_dump($s);
?>
--EXPECT--
array(6) {
  [0]=>
  string(6) "piece1"
  [1]=>
  string(6) "piece2"
  [2]=>
  string(6) "piece3"
  [3]=>
  string(6) "piece4"
  [4]=>
  string(6) "piece5"
  [5]=>
  string(6) "piece6"
}