--TEST--
swoole_coroutine: coro defer
--SKIPIF--
<?php require __DIR__ . '/../include/skipif.inc'; ?>
--FILE--
<?php
require __DIR__ . '/../include/bootstrap.php';
Swoole\Runtime::enableCoroutine();
go(function () {
    $obj = new class
    {
        public $resource;

        public function close()
        {
            $this->resource = null;
        }
    };
    defer(function () use ($obj) {
        $obj->close();
    });
    $obj->resource = $file = fopen(__FILE__, 'r+');
    defer(function () use ($obj) {
        assert(is_resource($obj->resource));
        fclose($obj->resource);
        echo "closed\n";
    });
    throw new Exception('something wrong');
    echo "never here\n";
});
?>
--EXPECTF--
closed

Warning: [Coroutine#1] Uncaught Exception: something wrong in %s/tests/swoole_coroutine/defer_close.php:23
Stack trace:
#0 {main}
  thrown in %s/tests/swoole_coroutine/defer_close.php on line 23
