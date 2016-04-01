--TEST--
If html sanitizer is working
--SKIPIF--
<?php if (!extension_loaded("csas")) print "skip"; ?>
--INI--
csas.enable=1
--FILE--
<?php
function main() {
    global $var;
    $a = "<a> csased string \\ & \" a\ra\na\va\fa\t</a>";
    html_sanitizer($a);
}

main();
echo $var;
?>
--EXPECTF--
&lt;a&gt; csased string &#39; &amp; &quote; a a a a a &lt;/a&gt;
