CREATE DATABASE IF NOT EXISTS cage;

USE cage;

CREATE TABLE IF NOT EXISTS trial_data (
    id INT(10) AUTO_INCREMENT PRIMARY KEY,
    cage INT(10),  
    program INT(10),
    mouse INT(10),
    trial INT(10),
    trialStart BIGINT,
    trialEnd BIGINT,
    duration BIGINT,
    result INT(10),
    wait4AP INT(10),
    waited INT(10),
    iteration INT(10)
);

CREATE TABLE IF NOT EXISTS arduino_events (
    id INT(10) AUTO_INCREMENT PRIMARY KEY,
    serialNumber VARCHAR(20),
    serialPort VARCHAR(20),
    timestamp BIGINT,
    event VARCHAR(50)
);
