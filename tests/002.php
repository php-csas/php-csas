<?php 
$a = "tainted string" . ".";
taint($a); //must use concat to make the string not a internal string(introduced in 5.4)

print $a;
$a .= '+';
$sql = "select * from {$a}";
file_put_contents("php://output", $a . "\n");
eval("return '$a';");
?>
