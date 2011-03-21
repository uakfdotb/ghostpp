CREATE TABLE admins (
  id int(11) NOT NULL auto_increment,
  botid int(11) NOT NULL,
  `name` varchar(15) NOT NULL,
  server varchar(100) NOT NULL,
  PRIMARY KEY  (id),
  KEY server (server)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

CREATE TABLE bans (
  id int(11) NOT NULL auto_increment,
  botid int(11) NOT NULL,
  server varchar(100) NOT NULL,
  `name` varchar(15) NOT NULL,
  ip varchar(15) NOT NULL,
  `date` datetime NOT NULL,
  gamename varchar(31) NOT NULL,
  admin varchar(15) NOT NULL,
  reason varchar(255) NOT NULL,
  PRIMARY KEY  (id),
  KEY server (server),
  KEY server_2 (server,`name`),
  KEY server_3 (server,`name`,ip)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

CREATE TABLE dotagames (
  id int(11) NOT NULL auto_increment,
  botid int(11) NOT NULL,
  gameid int(11) NOT NULL,
  winner int(11) NOT NULL,
  min int(11) NOT NULL,
  sec int(11) NOT NULL,
  PRIMARY KEY  (id),
  KEY gameid (gameid),
  KEY winner (winner)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

CREATE TABLE dotaplayers (
  id int(11) NOT NULL auto_increment,
  botid int(11) NOT NULL,
  gameid int(11) NOT NULL,
  colour int(11) NOT NULL,
  kills int(11) NOT NULL,
  deaths int(11) NOT NULL,
  creepkills int(11) NOT NULL,
  creepdenies int(11) NOT NULL,
  assists int(11) NOT NULL,
  gold int(11) NOT NULL,
  neutralkills int(11) NOT NULL,
  item1 char(4) NOT NULL,
  item2 char(4) NOT NULL,
  item3 char(4) NOT NULL,
  item4 char(4) NOT NULL,
  item5 char(4) NOT NULL,
  item6 char(4) NOT NULL,
  hero char(4) NOT NULL,
  newcolour int(11) NOT NULL,
  towerkills int(11) NOT NULL,
  raxkills int(11) NOT NULL,
  courierkills int(11) NOT NULL,
  PRIMARY KEY  (id),
  KEY gameid (gameid,colour),
  KEY colour (colour),
  KEY newcolour (newcolour)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

CREATE TABLE downloads (
  id int(11) NOT NULL auto_increment,
  botid int(11) NOT NULL,
  map varchar(100) NOT NULL,
  mapsize int(11) NOT NULL,
  `datetime` datetime NOT NULL,
  `name` varchar(15) NOT NULL,
  ip varchar(15) NOT NULL,
  spoofed int(11) NOT NULL,
  spoofedrealm varchar(100) NOT NULL,
  downloadtime int(11) NOT NULL,
  PRIMARY KEY  (id)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

CREATE TABLE gameplayers (
  id int(11) NOT NULL auto_increment,
  botid int(11) NOT NULL,
  gameid int(11) NOT NULL,
  `name` varchar(15) NOT NULL,
  ip varchar(15) NOT NULL,
  spoofed int(11) NOT NULL,
  reserved int(11) NOT NULL,
  loadingtime int(11) NOT NULL,
  `left` int(11) NOT NULL,
  leftreason varchar(100) NOT NULL,
  team int(11) NOT NULL,
  colour int(11) NOT NULL,
  spoofedrealm varchar(100) NOT NULL,
  PRIMARY KEY  (id),
  KEY gameid (gameid),
  KEY `name` (`name`),
  KEY colour (colour)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

CREATE TABLE games (
  id int(11) NOT NULL auto_increment,
  botid int(11) NOT NULL,
  server varchar(100) NOT NULL,
  map varchar(100) NOT NULL,
  `datetime` datetime NOT NULL,
  gamename varchar(31) NOT NULL,
  ownername varchar(15) NOT NULL,
  duration int(11) NOT NULL,
  gamestate int(11) NOT NULL,
  creatorname varchar(15) NOT NULL,
  creatorserver varchar(100) NOT NULL,
  PRIMARY KEY  (id)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

CREATE TABLE scores (
  id int(11) NOT NULL auto_increment,
  category varchar(25) NOT NULL,
  `name` varchar(15) NOT NULL,
  server varchar(100) NOT NULL,
  score double NOT NULL,
  PRIMARY KEY  (id),
  KEY category (category,`name`,server)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

CREATE TABLE w3mmdplayers (
  id int(11) NOT NULL auto_increment,
  botid int(11) NOT NULL,
  category varchar(25) NOT NULL,
  gameid int(11) NOT NULL,
  pid int(11) NOT NULL,
  `name` varchar(15) NOT NULL,
  flag varchar(32) NOT NULL,
  leaver int(11) NOT NULL,
  practicing int(11) NOT NULL,
  PRIMARY KEY  (id)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

CREATE TABLE w3mmdvars (
  id int(11) NOT NULL auto_increment,
  botid int(11) NOT NULL,
  gameid int(11) NOT NULL,
  pid int(11) NOT NULL,
  varname varchar(25) NOT NULL,
  value_int int(11) default NULL,
  value_real double default NULL,
  value_string varchar(100) default NULL,
  PRIMARY KEY  (id)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
