--TEST--
PDO: Testing if overwritten pdo functions are functioning correctly
--SKIPIF--
<?php
   if (!extension_loaded('pdo')) {
    die('skip mysqli extension not available.');
  }
  if (!extension_loaded('csas')){
    die('skip csas extension not avaialable.');
  }
  if (!extension_loaded("pdo_mysql")){
    die("skip must have mysql driver installed.");
  }
?>
--INI--
csas.enable=1
report_memleaks=Off
--ENV--
return <<<END
MYSQL_TEST_PASSWD=1234
END;
--FILE--
<?php
  require_once("connect.inc");
  try{
    $conn = new PDO("mysql:host=$host;dbname=mysql", $user, $passwd);
    // set the PDO error mode to exception
    $conn->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
    $sql = "CREATE DATABASE testDB";
    $conn->exec($sql);
    //select new database
    $conn->exec("USE testDB");
    //Initialize sample database for test.
    //create table
    $sql = "CREATE TABLE MyGuests (
    id INT(6) UNSIGNED AUTO_INCREMENT PRIMARY KEY,
    firstname VARCHAR(30) NOT NULL,
    lastname VARCHAR(30) NOT NULL,
    email VARCHAR(50),
    reg_date TIMESTAMP
    )";
    $conn->exec($sql);
    //add data to table
    $stmt = $conn->prepare("INSERT INTO MyGuests (firstname, lastname, email)
    VALUES (:firstname, :lastname, :email)");
    $stmt->bindParam(':firstname', $firstname);
    $stmt->bindParam(':lastname', $lastname);
    $stmt->bindParam(':email', $email);

    $firstname = "John";
    $lastname = "Doe";
    $email = "john@example.com";
    $stmt->execute();

    $firstname = "Hanako";
    $lastname = "Yamada";
    $email = "hanako@example.com";
    $stmt->execute();

    //test the overwritten functions
    $stmt = $conn->prepare("SELECT id, firstname, lastname FROM MyGuests");
    $stmt->execute();

    echo "Testing fetchAll:\n";
    var_dump($stmt->fetchALL(PDO::FETCH_ASSOC));
    $stmt->execute();
    echo "Testing fetch:\n";
    var_dump($stmt->fetch(PDO::FETCH_ASSOC));
    var_dump($stmt->fetch(PDO::FETCH_NUM));
    $stmt->execute();
    echo "Testing fetchColumn:\n";
    var_dump($stmt->fetchColumn(1));
    echo "Testing fetchObject:\n";
    var_dump($stmt->fetchObject());

    $conn->exec("DROP DATABASE testDB");
  }
  catch(PDOException $e)
  {
    die("Error: " . $e->getMessage());
  }
  $conn = null;
?>
--EXPECTF--
Testing fetchAll:
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
Testing fetch:
array(3) {
  ["id"]=>
  string(1) "1"
  ["firstname"]=>
  string(4) "John"
  ["lastname"]=>
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
Testing fetchColumn:
string(4) "John"
Testing fetchObject:
object(stdClass)#3 (3) {
  ["id"]=>
  string(1) "2"
  ["firstname"]=>
  string(6) "Hanako"
  ["lastname"]=>
  string(6) "Yamada"
}