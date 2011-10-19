<?php

require_once DRUPAL_ROOT . '/includes/database/mysql/database.inc';

class DatabaseConnection_mysqltungsten extends DatabaseConnection_mysql {
  public function __construct(array $connection_options = array()) {
    // This driver defaults to transaction support, except if explicitly passed FALSE.
    $this->transactionSupport = !isset($connection_options['transactions']) || ($connection_options['transactions'] !== FALSE);

    // MySQL never supports transactional DDL.
    $this->transactionalDDLSupport = FALSE;

    $this->connectionOptions = $connection_options;

    // The DSN should use either a socket or a host/port.
    if (isset($connection_options['unix_socket'])) {
      $dsn = 'mysql:unix_socket=' . $connection_options['unix_socket'];
    }
    else {
      // Default to TCP connection on port 3306.
      $dsn = 'mysql:host=' . $connection_options['host'] . ';port=' . (empty($connection_options['port']) ? 3306 : $connection_options['port']);
    }
    $dsn .= ';dbname=' . $connection_options['database'];
    DatabaseConnection::__construct($dsn, $connection_options['username'], $connection_options['password'], array(
      // So we don't have to mess around with cursors and unbuffered queries by default.
      PDO::MYSQL_ATTR_USE_BUFFERED_QUERY => TRUE,
      // Because MySQL's prepared statements skip the query cache, because it's dumb.
      PDO::ATTR_EMULATE_PREPARES => TRUE,
      // Force column names to lower case.
      PDO::ATTR_CASE => PDO::CASE_LOWER,
    ));

    // Force MySQL to use the UTF-8 character set. Also set the collation, if a
    // certain one has been set; otherwise, MySQL defaults to 'utf8_general_ci'
    // for UTF-8.
    if (!empty($connection_options['collation'])) {
      $this->exec('SET NAMES utf8 COLLATE ' . $connection_options['collation']);
    }
    else {
      $this->exec('SET NAMES utf8');
    }

    // Work around a tungsten-replicator bug that prevents replication with
    // ANSI_QUOTES sql_mode. Here we unset ANSI_QUOTES and allow init_commands
    // to be overridden.
    $connection_options += array(
      'init_commands' => array(),
    );
    $connection_options['init_commands'] += array(
      'sql_mode' => "SET sql_mode='REAL_AS_FLOAT,PIPES_AS_CONCAT,IGNORE_SPACE,STRICT_TRANS_TABLES,STRICT_ALL_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_AUTO_CREATE_USER'",
    );
    // Set connection options.
    $this->exec(implode('; ', $connection_options['init_commands']));
  }
  function driver() {
    return 'mysqltungsten';
  }
}

