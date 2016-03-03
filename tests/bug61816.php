<?php
$a = "tainted string" . ".\n";
taint($a);
$b = array("");
$b[0] .= $a;
var_dump(is_tainted($b[0]));
$c = new stdClass();
$c->foo = "this is";
$c->foo .= $b[0];
echo $b[0];  // Segmentation fault
var_dump(is_tainted($c->foo));
?>
