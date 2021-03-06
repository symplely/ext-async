--TEST--
Channel group send into closed channel.
--SKIPIF--
<?php require __DIR__ . '/skipif.inc'; ?>
--FILE--
<?php

namespace Concurrent;

$group = new ChannelGroup([
    'A' => ($a = new Channel()),
    'B' => ($b = new Channel())
], true);

$a->close();

var_dump(count($group));

try {
    var_dump($group->send(1, 0));
} catch (ChannelClosedException $e) {
    var_dump($e->getMessage());
}

var_dump(count($group));

$b->close();

var_dump(count($group));

try {
    var_dump($group->send(2));
} catch (ChannelClosedException $e) {
    var_dump($e->getMessage());
}

var_dump(count($group));

$group = new ChannelGroup([
    $c = new Channel(),
]);

Task::async(function () use ($c) {
    (new Timer(20))->awaitTimeout();

    $c->close();
});

var_dump(count($group));

try {
    var_dump($group->send(3));
} catch (ChannelClosedException $e) {
    var_dump($e->getMessage());
}

var_dump(count($group));

--EXPECT--
int(2)
string(23) "Channel has been closed"
int(1)
int(1)
string(23) "Channel has been closed"
int(0)
int(1)
string(23) "Channel has been closed"
int(0)
