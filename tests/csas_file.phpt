--TEST--
csas_file()
--SKIPIF--
<?php if (!extension_loaded("csas")) print "skip"; ?>
--INI--
csas.enable=1
--FILE--

<?php
$fp = fopen("test.txt", "w");
if (!$fp) {
    echo 'Could not open testfile.txt';
}
$txt = "Hello World. Testing testing!\nAnother day, another line.\nIf the array picks up this line,\nthen is it a pickup line?";
fwrite($fp, $txt);
fclose($fp);
print_r(file("test.txt"));
?>

--EXPECTF--
Array
(
    [0] => Hello World. Testing testing!

    [1] => Another day, another line.

    [2] => If the array picks up this line,

    [3] => then is it a pickup line?
)