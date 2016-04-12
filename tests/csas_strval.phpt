--TEST--
csas_strval() function - basic test for csas_strval()
--FILE--
<?php
  class StrValTester
  {
    public function __toString()
    {
        return make_unsafe(__CLASS__);
    }
  }
  $var = new StrValTester;
  make_unsafe($var);
  $s = csas_strval(var);
  var_dump($s);
  $safety = test_csas_get_safety($s);
  if ($safety != 0xFFFFFFFF){
    var_dump("True")
  }
?>
--EXPECT--
string(12) "StrValTester"
string (4) "True"