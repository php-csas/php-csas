--TEST--
printf: Testing if overwritten printf functions is functioning correctly
--SKIPIF--
<?php
  if (!extension_loaded('csas')){
    die('skip csas extension not avaialable.');
  }
?>
--INI--
csas.enable=1
report_memleaks=Off
mysql.default_socket=/var/run/mysqld/mysqld.sock
--FILE--
<?php
$a = "csased string";
csas($a);
printf("<a> %s </a>\nnext line", $a);
printf("\n");
var_dump(is_csased($a));
?>
--EXPECT--
&lt;a&gt; csased string &lt;/a&gt; next line
bool(true)