<?php
$start = microtime(true);
for ($i=0; $i<10000; $i++) {
	$enc = json_encode($_SERVER);
}
printf("json_encode: %s\n", microtime(true)-$start);
$legacy = $enc;

$start = microtime(true);
for ($i=0; $i<10000; $i++) {
	$enc = jsson_encode($_SERVER);
}
$new = $enc;
printf("jsson_encode: %s\n", microtime(true)-$start);

$start = microtime(true);
for ($i=0; $i<10000; $i++) {
	$l=json_decode($legacy);
}
printf("json_decode: %s\n", microtime(true)-$start);

$start = microtime(true);
for ($i=0; $i<10000; $i++) {
	$n=jsson_decode($new);
	var_dump($n);
}
printf("jsson_decode: %s\n", microtime(true)-$start);
var_dump(count($n), count($l));

?>
