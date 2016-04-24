--TEST--
csas_fgetc()
--SKIPIF--
<?php if (!extension_loaded("csas")) print "skip"; ?>
--INI--
csas.enable=1
report_memleaks=Off
--FILE--
<?php
$fp = fopen("testfile.txt", "w");
if (!$fp) {
    echo 'Could not open testfile.txt';
}
$txt = "John Doe Jane Doe";
fwrite($fp, $txt);
fclose($fp);
$out = "";
$fp = fopen("testfile.txt", 'r');
if (!$fp) {
    echo 'Could not open testfile.txt';
}
while (false !== ($char = fgetc($fp))) {
  $out .= $char;
}
fclose($fp);
echo $out;
?>

--EXPECTF--
Warning: main(): Attempt to echo a string that might be csased in %scsas_fgetc.php on line %d
John Doe Jane Doe