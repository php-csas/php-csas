--TEST--
Test if socket_recvfrom() receives data sent by socket_sendto() via IPv4 UDP
--SKIPIF--
<?php
if (!extension_loaded('sockets')) {
    die('SKIP The sockets extension is not loaded.');
}
if (!extension_loaded("csas")){
    die("skip csas extension not available");
}
?>
--INI--
csas.enable=1
--FILE--
<?php
  /* Setup socket server */
  $server = socket_create(AF_INET, SOCK_STREAM, getprotobyname('tcp'));
  if (!$server) {
    die('Unable to create AF_INET socket [server]');
  }
  $bound = false;
  for($port = 31337; $port < 31357; ++$port) {
    if (socket_bind($server, '127.0.0.1', $port)) {
      $bound = true;
      break;
    }
  }
  if (!$bound) {
    die("Unable to bind to 127.0.0.1");
  }
  if (!socket_listen($server, 2)) {
    die('Unable to listen on socket');
  }

  /* Connect to it */
  $client = socket_create(AF_INET, SOCK_STREAM, getprotobyname('tcp'));
  if (!$client) {
    die('Unable to create AF_INET socket [client]');
  }
  if (!socket_connect($client, '127.0.0.1', $port)) {
    die('Unable to connect to server socket');
  }
  /* Accept that connection */
  $socket = socket_accept($server);
  if (!$socket) {
    die('Unable to accept connection');
  }
  socket_send($client, "ABCdef123\n", 10, 0);
  $data = "1234567890";
  $host = "127.0,0,1";
  socket_recvfrom($socket, $data, 10, MSG_WAITALL, $host, $port);
  var_dump($data);
  socket_close($client);
  socket_close($socket);
  socket_close($server);
?>
--EXPECTF--
string(10) "ABCdef123
"