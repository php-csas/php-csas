--TEST--
html_quoted_sanitize: checking functionality
--SKIPIF--
<?php if (!extension_loaded("csas")) print "skip"; ?>
--INI--
csas.enable=1
--FILE--
<?php
  $a = "&\"'<>\r\n\v\f\t";
  $a = html_quoted_sanitize($a);
  var_dump($a);
?>
--EXPECTF--
string(29) "&amp;&quot;&#39;&lt;&gt;     "
