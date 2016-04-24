--TEST--
csas_file_get_contents()
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
$txt = "This is a test file with test text.";
fwrite($fp, $txt);
fclose($fp);
echo file_get_contents("test.txt");
?>

--EXPECTF--
Warning: main(): Attempt to echo a string that might be csased in %scsas_file_get_contents.php on line %d
This is a test file with test text.