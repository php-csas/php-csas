--TEST--
html_unquoted_sanitizer: check functionality
--SKIPIF--
<?php if (!extension_loaded("csas")) print "skip"; ?>
--INI--
csas.enable=1
--FILE--
<?php
$a = "&\"'<> `=\f\t\n";
$a = html_unquoted_sanitize($a);
echo $a;
?>
--EXPECTF--
&amp;&quot;&#39;&lt;&gt;&#32;&#96;&#61;&#12;&#9;&#10;