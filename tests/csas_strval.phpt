--TEST--
csas_strval() function - basic test for csas_strval()
--FILE--
<?php
  class StrValTest
  {
      public function __toString()
      {
          return __CLASS__;
      }
  }

  $var = new StrValTest;
  $s = strval($var);
  var_dump($s);
?>
--EXPECT--
string(10) "StrValTest"