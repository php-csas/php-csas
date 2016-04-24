--TEST--
csas_fscanf()
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
$txt = "javier  argonaut        pe\nhiroshi sculptor        jp\nrobert  slacker us\nluigi   florist it";
fwrite($fp, $txt);
fclose($fp);

$handle = fopen("test.txt", "r");
while ($userinfo = fscanf($handle, "%s\t%s\t%s\n")) {
    list ($name, $profession, $countrycode) = $userinfo;
    var_dump($name);
}
fclose($handle);
?>

--EXPECTF--
string(6) "javier"
string(7) "hiroshi"
string(6) "robert"
string(5) "luigi"