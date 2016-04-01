<?php 

$str = 'a,' . 'b';
csas($str);
$a = explode(',', $str);
while (list($key, $val) = each($a)) {
    echo $val;
}

?>
