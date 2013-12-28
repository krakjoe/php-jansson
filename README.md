php-jansson
===========

jansson is a pretty fast json encoder/decoder written in C: https://github.com/akheron/jansson

```php
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
	json_decode($legacy);
}
printf("json_decode: %s\n", microtime(true)-$start);

$start = microtime(true);
for ($i=0; $i<10000; $i++) {
	jsson_decode($new);
}
printf("jsson_decode: %s\n", microtime(true)-$start);
?>
```

```
json_encode: 1.1663799285889
jsson_encode: 0.96577715873718
json_decode: 2.0289180278778
jsson_decode: 1.673702955246
```

Installation
============

```
phpize
./configure --with-jansson=/usr
make install
```

API
===

```
string jsson_encode(mixed $variable);
mixed jsson_decode(string $json);
```

*Note: there is an extra 's' in the function names to avoid clashing*
