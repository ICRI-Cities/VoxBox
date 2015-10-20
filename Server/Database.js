/**
 * Created by sarahg on 16/10/15.
 */

var mysql = require('mysql');

var host = 'localhost';
var user = 'root';
var pswrd = 'root';

/**
 * @constructor
 */
var Database = function(){};

Database.prototype.StoreToDB = function(db, query, storeset){
    var connection = mysql.createConnection({
        host:host,
        user: user,
        password: pswrd,
        database: db
    });
    connection.connect();

    connection.query(query, storeset, function(err, result){
        if(err) throw err;
        connection.end();
    });
};

Database.prototype.GetFromDB = function(db, query, callback){
    var connection = mysql.createConnection({
        host: host,
        user: user,
        password: pswrd,
        database: db
    });
    connection.connect();

    connection.query(query, function(err, result){
        if(err) {
            throw err;
        } else {
            var values = result;
            //console.log(values);
            callback(values);
        }
        connection.end();
    });
};

module.exports = Database;