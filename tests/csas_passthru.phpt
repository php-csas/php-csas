--TEST--
passthru: Testing if overwritten passthru functions is functioning correctly
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
passthru('echo "<a> test </a>"');
?>
--EXPECT--
<a> test </a>