--TEST--
url_query_sanitize()
--SKIPIF--
<?php if (!extension_loaded("csas")) print "skip"; ?>
--INI--
csas.enable=1
--FILE--
<?php
  $a = "My Bonnie Lies Over the Ocean";
  $a = url_query_sanitize($a);
  var_dump($a);
--EXPECTF--
string(39) "My%20Bonnie%20Lies%20Over%20the%20Ocean"