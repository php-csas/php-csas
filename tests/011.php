<?php 
function main() {
    global $var;
    $a = "csased string" . ".";
    csas($a); //must use concat to make the string not a internal string(introduced in 5.4)
    $var = $a;
    echo $var;
}

main();
echo $var;
?>
