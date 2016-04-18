--TEST--
csas_fgetss()
--SKIPIF--
<?php if (!extension_loaded("csas")) print "skip"; ?>
--INI--
csas.enable=1
report_memleaks=Off
--FILE--

<?php
$fp = fopen("test.htm", "w");
if (!$fp) {
    echo 'Could not open testfile.txt';
}
$txt = "<p><b>This is a paragraph.</b></p>";
fwrite($fp, $txt);
fclose($fp);
$file = fopen("test.htm","r");
echo fgetss($file,1024);
fclose($file);
?>

--EXPECTF--
Warning: main(): Attempt to echo a string that might be csased in %scsas_fgetss.php on line %d
This is a paragraph.