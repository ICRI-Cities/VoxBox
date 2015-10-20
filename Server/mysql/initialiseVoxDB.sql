create database voxDB;
use voxDB;

create table demographics(
	date varchar(255) NOT NULL,
    time varchar(255) NOT NULL,	
    age int,
    comeFrom int,
    cameWith int,
    gender int,
    PRIMARY KEY (date, time)
);

create table mood(
    date varchar(255) NOT NULL,
    time varchar(255) NOT NULL,
    slide1 int,
    slide2 int,
    slide3 int,
    slide4 int,
    slide5 int,
    PRIMARY KEY (date, time)
);

create table crowd(
    date varchar(255) NOT NULL,
    time varchar(255) NOT NULL,
    knob1 int,
    knob2 int,
    knob3 int,
    PRIMARY KEY (date, time)
);

create table event(
    date varchar(255) NOT NULL,
    time varchar(255) NOT NULL,
    spin1 int,
    spin2 int,
    PRIMARY KEY (date, time)
);

create table phone(
    date varchar(255) NOT NULL,
    time varchar(255) NOT NULL,
    PRIMARY KEY (date, time)
);

create table engineer(
    date varchar(255) NOT NULL,
    time varchar(255) NOT NULL,
    type int,
    PRIMARY KEY (date, time)
);
