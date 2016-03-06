<?php
	$a = trim($_GET['a']);

	$output    = "Welcome, {$a} !!!";
	$var       = "output";

	echo $output;

	print $$var;
?>
