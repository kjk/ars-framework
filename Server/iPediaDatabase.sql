# Connection: rabban
# Host: rabban
# Saved: 2004-03-12 20:42:42
# 
# Host: rabban
# Database: ipedia
# Table: 'cookies'
# 
CREATE TABLE `cookies` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `cookie` varchar(32) binary NOT NULL default '',
  `device_info_token` varchar(255) NOT NULL default '',
  `issue_date` timestamp(14) NOT NULL,
  PRIMARY KEY  (`id`),
  UNIQUE KEY `cookie_unique` (`cookie`)
) TYPE=MyISAM; 

# Host: rabban
# Database: ipedia
# Table: 'definitions'
# 
CREATE TABLE `definitions` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `term` varchar(255) NOT NULL default '',
  `definition` mediumtext NOT NULL,
  `last_modified` timestamp(14) NOT NULL,
  PRIMARY KEY  (`id`),
  UNIQUE KEY `term_index` (`term`)
) TYPE=MyISAM; 

# Host: rabban
# Database: ipedia
# Table: 'registered_users'
# 
CREATE TABLE `registered_users` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `cookie_id` int(10) unsigned default '0',
  `user_name` varchar(255) NOT NULL default '',
  `serial_number` varchar(255) binary NOT NULL default '',
  `registration_date` timestamp(14) NOT NULL,
  PRIMARY KEY  (`id`)
) TYPE=MyISAM; 

# Host: rabban
# Database: ipedia
# Table: 'term_requests'
# 
CREATE TABLE `term_requests` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `requested_term` varchar(255) NOT NULL default '',
  `cookie` varchar(32) binary NOT NULL default '',
  `transaction_id` int(10) unsigned NOT NULL default '0',
  `cookie_id` int(10) unsigned NOT NULL default '0',
  `definition_id` int(10) unsigned NOT NULL default '0',
  `request_date` timestamp(14) NOT NULL,
  PRIMARY KEY  (`id`)
) TYPE=MyISAM; 

