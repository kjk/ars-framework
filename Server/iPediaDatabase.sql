# Connection: localhost
# Host: localhost
# Saved: 2004-03-16 19:49:06
# 
# Host: localhost
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

# Host: localhost
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

# Host: localhost
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

# Host: localhost
# Database: ipedia
# Table: 'requests'
# 
CREATE TABLE `requests` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `client_ip` int(10) unsigned NOT NULL default '0',
  `transaction_id` int(10) unsigned NOT NULL default '0',
  `has_get_cookie_field` tinyint(1) NOT NULL default '0',
  `cookie_id` int(10) unsigned default '0',
  `has_register_field` tinyint(1) NOT NULL default '0',
  `requested_term` varchar(255) default '',
  `error` int(10) unsigned NOT NULL default '0',
  `definition_id` int(10) unsigned default '0',
  PRIMARY KEY  (`id`)
) TYPE=MyISAM; 

