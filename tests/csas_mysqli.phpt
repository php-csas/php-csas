--TEST--
mysqli: Testing if overwritten mysqli functions are functioning correctly
--SKIPIF--
<?php
   if (!extension_loaded('mysqli')) {
    die('skip mysqli extension not available.');
  }
  if (!extension_loaded('csas')){
    die('skip csas extension not avaialable.');
  }
?>
--INI--
csas.enable=1
report_memleaks=Off
mysql.default_socket=/var/run/mysqld/mysqld.sock
--FILE--
<?php
  require_once("connect.inc");
  // Create connection
  $conn = new mysqli($host, $user, $passwd, NULL, $port, $socket);
  // Check connection
  if ($conn->connect_errno > 0) {
      die("Connection failed: " . $conn->connect_error);
  }
  // skip if database cannot be created
  $sql = "CREATE DATABASE testDB";
  if ($conn->query($sql) === FALSE) {
      die("Error creating database, check mysqli defaults: " . $conn->error);
  }
  //Initialize sample database for test.
  $conn->select_db("testDB");
  //create table
  $sql = "CREATE TABLE MyGuests (
  id INT(6) UNSIGNED AUTO_INCREMENT PRIMARY KEY,
  firstname VARCHAR(30) NOT NULL,
  lastname VARCHAR(30) NOT NULL,
  email VARCHAR(50),
  reg_date TIMESTAMP
  )";

  if ($conn->query($sql) === FALSE) {
      die("Error creating table: " . $conn->error);
  }
  //Insert data into table
  $sql = "INSERT INTO MyGuests (firstname, lastname, email)
  VALUES ('John', 'Doe', 'john@example.com')";
  if ($conn->query($sql) === FALSE) {
      die("Error: " . $sql . "<br>" . $conn->error);
  }
  $sql = "INSERT INTO MyGuests (firstname, lastname, email)
  VALUES ('Hanako', 'Yamada', 'hanako@example.com')";
  if ($conn->query($sql) === FALSE) {
      die("Error: " . $sql . "<br>" . $conn->error);
  }
  //get result from db
  $sql = "SELECT id, firstname, lastname FROM MyGuests";
  if ($result = $conn->query($sql)) {
    //test mysqli_result::fetch_assoc
    echo ("Testing fetch assoc:\n");
    var_dump($result->fetch_assoc());
    //test mysqli_fetch_assoc;
    var_dump(mysqli_fetch_assoc($result));
    //test mysqli_result::fetch_array
    $result->data_seek(0);
    echo ("Testing fetch array:\n");
    var_dump($result->fetch_array(MYSQLI_NUM));
    //test mysqli_fetch_array
    var_dump(mysqli_fetch_array($result, MYSQLI_NUM));
    //test mysqli_result::fetch_all
    $result->data_seek(0);
    echo ("Testing fetch all:\n");
    var_dump($result->fetch_all(MYSQLI_ASSOC));
    //test mysqli_fetch_all
    $result->data_seek(0);
    var_dump(mysqli_fetch_all($result, MYSQLI_ASSOC));
    //test mysqli_result::fetch_object
    $result->data_seek(0);
    echo ("Testing fetch object:\n");
    var_dump(($result->fetch_object()));
    //test mysqli_fetch_object
    var_dump(mysqli_fetch_object($result));
    //test mysqli_result::fetch_row
    $result->data_seek(0);
    echo ("Testing fetch row:\n");
    var_dump($result->fetch_row());
    //test mysqli_fetch_row
    var_dump(mysqli_fetch_row($result));
    $result->free();
  }else{
    die("Couldn't get result from test DB");
  }
  if($conn->query("DROP DATABASE testDB") === FALSE){
    die('Could not delete database: ' . mysql_error()->error);
  }
  $conn->close();
?>
--EXPECTF--
Testing fetch assoc:
array(3) {
  ["id"]=>
  string(1) "1"
  ["firstname"]=>
  string(4) "John"
  ["lastname"]=>
  string(3) "Doe"
}
array(3) {
  ["id"]=>
  string(1) "2"
  ["firstname"]=>
  string(6) "Hanako"
  ["lastname"]=>
  string(6) "Yamada"
}
Testing fetch array:
array(3) {
  [0]=>
  string(1) "1"
  [1]=>
  string(4) "John"
  [2]=>
  string(3) "Doe"
}
array(3) {
  [0]=>
  string(1) "2"
  [1]=>
  string(6) "Hanako"
  [2]=>
  string(6) "Yamada"
}
Testing fetch all:
array(2) {
  [0]=>
  array(3) {
    ["id"]=>
    string(1) "1"
    ["firstname"]=>
    string(4) "John"
    ["lastname"]=>
    string(3) "Doe"
  }
  [1]=>
  array(3) {
    ["id"]=>
    string(1) "2"
    ["firstname"]=>
    string(6) "Hanako"
    ["lastname"]=>
    string(6) "Yamada"
  }
}
array(2) {
  [0]=>
  array(3) {
    ["id"]=>
    string(1) "1"
    ["firstname"]=>
    string(4) "John"
    ["lastname"]=>
    string(3) "Doe"
  }
  [1]=>
  array(3) {
    ["id"]=>
    string(1) "2"
    ["firstname"]=>
    string(6) "Hanako"
    ["lastname"]=>
    string(6) "Yamada"
  }
}
Testing fetch object:
object(stdClass)#4 (3) {
  ["id"]=>
  string(1) "1"
  ["firstname"]=>
  string(4) "John"
  ["lastname"]=>
  string(3) "Doe"
}
object(stdClass)#4 (3) {
  ["id"]=>
  string(1) "2"
  ["firstname"]=>
  string(6) "Hanako"
  ["lastname"]=>
  string(6) "Yamada"
}
Testing fetch row:
array(3) {
  [0]=>
  string(1) "1"
  [1]=>
  string(4) "John"
  [2]=>
  string(3) "Doe"
}
array(3) {
  [0]=>
  string(1) "2"
  [1]=>
  string(6) "Hanako"
  [2]=>
  string(6) "Yamada"
}