DROP DATABASE IF EXISTS ipedia;
CREATE DATABASE ipedia;

GRANT ALL ON enwiki.* TO 'ipedia'@'localhost' IDENTIFIED BY 'ipedia'; 
GRANT ALL ON ipedia.* TO 'ipedia'@'localhost' IDENTIFIED BY 'ipedia'; 

