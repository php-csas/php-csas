--TEST--
url_general_sanitize()'
--SKIPIF--
<?php if (!extension_loaded("csas")) print "skip"; ?>
--INI--
csas.enable=1
--FILE--
<?php
  $a = "&\"'<>\r\n\v\f\t";
  $a = url_general_sanitize($a);
  var_dump($a);
?>
--EXPECTF--
string(24) "&amp;&quot;&#39;&lt;&gt;"