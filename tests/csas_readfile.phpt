--TEST--
csas_readfile
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
$txt = "<a> This is a test file with test text. </a>";
fwrite($fp, $txt);
fclose($fp);
readfile("test.txt");
?>
--EXPECTF--
&lt;a&gt; This is a test file with test text. &lt;/a&gt;
