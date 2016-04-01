<?php
$a = "csased string" . ".";
csas($a); //must use concat to make the string not a internal string(introduced in 5.4)

$b = isset($a)? $a : 0;
echo $b;

$b .= isset($a)? "xxxx" : 0; //a knew mem leak
echo $b;
?>
