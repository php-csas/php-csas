--TEST--
url_start_sanitize()
--SKIPIF--
<?php if (!extension_loaded("csas")) print "skip"; ?>
--INI--
csas.enable=1
--FILE--
<?php
$a = "http://test.com/test.html\n";
$a = url_start_sanitize($a);
echo $a;
$a = "https://test.com/test.html\n";
$a = url_start_sanitize($a);
echo $a;
$a = "mailto:test.com/test.html";
$a = url_start_sanitize($a);
echo $a;
$a= "abc://test.com/test.html";
$a = url_start_sanitize($a);
echo $a;
echo "\n";
$a= "abc:test.com/test.html";
$a = url_start_sanitize($a);
echo $a;
$a= "abcdefg";
$a = url_start_sanitize($a);
echo $a;
?>
--EXPECTF--
http://test.com/test.html
https://test.com/test.html
mailto:test.com/test.html