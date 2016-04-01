--TEST--
If url start sanitizer is working
--SKIPIF--
<?php if (!extension_loaded("csas")) print "skip"; ?>
--INI--
csas.enable=1
--FILE--
<?php
function main() {
    global $var;
    $a = "http://test.com/test.html";
    url_start_sanitizer($a);
    $a = "https://test.com/test.html";
    url_start_sanitizer($a);
    $a = "mailto:test.com/test.html";
    url_start_sanitizer($a);
    $a = "abc://test.com/test.html";
    url_start_sanitizer($a);
    $a = "abcdefgh:";
    url_start_sanitizer($a);
    $a = "abc:";
    url_start_sanitizer($a);
    url_start_sanitizer($a);
    $a = "abc:test.com/test.html";
}

main();
echo $var;
?>
--EXPECTF--
http://test.com/test.html
https://test.com/test.html
mailto:test.com/test.html
test.com/test.html


test.com/test.html

