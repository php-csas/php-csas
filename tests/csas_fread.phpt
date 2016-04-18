--TEST--
csas_fread()
--SKIPIF--
<?php if (!extension_loaded("csas")) print "skip"; ?>
--INI--
csas.enable=1
report_memleaks=Off
--FILE--
<?php
$fp = fopen("test.txt", "w");
if (!$fp) {
    echo 'Could not open test.txt';
}
$txt = "Hello World. Testing testing!
Another day, another line.
If the array picks up this line,
then is it a pickup line?";
fwrite($fp, $txt);
$file = fopen("test.txt","r");
$s = fread($file,filesize("test.txt"));
fclose($file);
echo $s;
?>

--EXPECTF--
Warning: main(): Attempt to echo a string that might be csased in %scsas_fread.php on line %d
Hello World. Testing testing! Another day, another line. If the array picks up this line, then is it a pickup line?