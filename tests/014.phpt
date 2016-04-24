--TEST--
If js sanitizer is working
--SKIPIF--
<?php if (!extension_loaded("csas")) print "skip"; ?>
--INI--
csas.enable=1
--FILE--
<?php
  $a = "\0\"\\'=&<>\f\t\n\r\b\v";
  $a = js_sanitize($a);
  var_dump($a);
--EXPECTF--
string(45) "\x00\x22\\\x27\x3d\x26\x3c\x3e\f\t\n\r\\b\x0b"