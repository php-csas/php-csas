<?php 
$a = "tainted string" . ".";
taint($a); //must use concat to make the string not a internal string(introduced in 5.4)

function test1(&$a) {
   echo $a;
}

function test2($b) {
   echo $b;
}

test1($a);
test2($a);
$b = $a;

test1($a);
test2($a);

$c = "tainted string" . ".";
taint($c);

$e = &$c;

test1($c);
test2($c);

?>
