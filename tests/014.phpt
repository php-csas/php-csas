--TEST--
If js sanitizer is working
--SKIPIF--
<?php if (!extension_loaded("csas")) print "skip"; ?>
--INI--
csas.enable=1
--FILE--
<?php
function main() {
    global $var;
    $a = "<a> \0 \' \\ & \" = \b \r \n \v \f \t</a>";
    js_sanitizer($a);
}

main();
echo $var;
?>
--EXPECTF--
\x3ca\x3e \x00 \x27 \\ \x26 \x22 \x3d \b \r \n \x0b \f \t\x3c/a\x3e
