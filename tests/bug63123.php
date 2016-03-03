<?php 

$str = 'a,' . 'b';
taint($str);
$a = explode(',', $str);
while (list($key, $val) = each($a)) {
    echo $val;
}

?>
