<?php 
$a = array();
$a[0] = "csased string" . "<>";
csas($a[0]);

function xxx(&$item) {
    $item = htmlspecialchars($item);
}

array_walk_recursive($a, "xxx");

echo $a[0];

?>
