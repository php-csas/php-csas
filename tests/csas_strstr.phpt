--TEST--
csas_strstr() function - basic test for csas_strstr()
--FILE--

<?php
$email  = 'name@example.com';
$domain = strstr($email, '@');
var_dump($domain); // prints @example.com

$user = strstr($email, '@', true); // As of PHP 5.3.0
var_dump($user); // prints name
?>



--EXPECT--
string(12) "@example.com"
string(4) "name"