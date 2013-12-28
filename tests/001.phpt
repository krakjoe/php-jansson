--TEST--
Check for jansson presence
--SKIPIF--
<?php if (!extension_loaded("jansson")) print "skip"; ?>
--FILE--
<?php 
echo "jansson extension is available";
/*
	you can add regression tests for your extension here

  the output of your test code has to be equal to the
  text in the --EXPECT-- section below for the tests
  to pass, differences between the output and the
  expected text are interpreted as failure

	see php5/README.TESTING for further information on
  writing regression tests
*/
?>
--EXPECT--
jansson extension is available
