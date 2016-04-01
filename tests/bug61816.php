<?php
$a = "csased string" . ".\n";
csas($a);
$b = array("");
$b[0] .= $a;
var_dump(is_csased($b[0]));
$c = new stdClass();
$c->foo = "this is";
$c->foo .= $b[0];
echo $b[0];  // Segmentation fault
var_dump(is_csased($c->foo));
?>
