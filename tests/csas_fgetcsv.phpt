--TEST--
csas_fgetcsv()
--SKIPIF--
<?php if (!extension_loaded("csas")) print "skip"; ?>
--INI--
csas.enable=1
--FILE--

<?php
$fp = fopen("test.csv", "w");
if (!$fp) {
    echo 'Could not open testfile.txt';
}
$txt = "Kai Jim,Refsnes,Stavanger,Norway\n";
fwrite($fp, $txt);
$txt = "Hege,Refsnes,Stavanger,Norway\n";
fwrite($fp, $txt);
fclose($fp);
$file = fopen("test.csv","r");

while(! feof($file))
  {
  print_r(fgetcsv($file));
  }

fclose($file);
?>

--EXPECTF--
Array
(
    [0] => Kai Jim
    [1] => Refsnes
    [2] => Stavanger
    [3] => Norway
)
Array
(
    [0] => Hege
    [1] => Refsnes
    [2] => Stavanger
    [3] => Norway
)