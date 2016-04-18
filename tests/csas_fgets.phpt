--TEST--
csas_fgets()
--SKIPIF--
<?php if (!extension_loaded("csas")) print "skip"; ?>
--INI--
csas.enable=1
report_memleaks=Off
--FILE--

<?php
$fp = fopen("test.txt", "w");
if (!$fp) {
    echo 'Could not open testfile.txt';
}
$txt = "Hello, this is a test file.\nThere are three lines here.\nThis is the last line.";
fwrite($fp, $txt);
fclose($fp);
$file = fopen("test.txt","r");
echo fgets($file);
fclose($file);
?>

--EXPECTF--
Warning: main(): Attempt to echo a string that might be csased in %scsas_fgets.php on line %d
Hello, this is a test file.