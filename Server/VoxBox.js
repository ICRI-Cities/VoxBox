/**
 * Created by sarahg on 16/10/15.
 */

var Database = require('./Database');
var db = new Database();
var dbName = "voxDB";

ClientTypes = {
    DEMOGRAPHICS : 0,
    MOOD : 1,
    CROWD : 2,
    EVENT : 3,
    PHONE : 4
}

var clients = [
    [1, ClientTypes.DEMOGRAPHICS, 5, processDemo],
    [3, ClientTypes.CROWD, 7, processCrowd],
    [2, ClientTypes.MOOD, 6, processMood],
    [4, ClientTypes.EVENT, 3, processEvent],
    [5, ClientTypes.PHONE, 2, processPhone]
];

function handlePing(){
    var timestamp = new Date();
    var date = timestamp.getFullYear()+"-"+(timestamp.getMonth()+1)+"-"+timestamp.getDate();
    var time = timestamp.toTimeString();
    console.log(date+" | "+time+" : Voxbox still connected");
}

function handleSlaveRequest(socket){
    console.log("returning slave list to VoxBox...");
    //return slave list

    var list = '';
    for(var i=0; i<clients.length; i++){
        //get client ID
        var clientID = clients[i][0];
        var h_id = Math.floor(clientID/100);
        if(h_id > 0){
            clientID = clientID-(h_id*100);
        }
        var t_id = Math.floor(clientID/10);
        if(t_id > 0){
            clientID = clientID-(t_id*10);
        }
        var u_id = clientID;
        //get data length
        var dataLength = clients[i][2];
        var h_length = Math.floor(dataLength/100);
        if(h_length > 0){
            dataLength = dataLength-(h_length*100);
        }
        var t_length = Math.floor(dataLength/10);
        if(t_length > 0){
            dataLength = dataLength-(t_length*10);
        }
        var u_length = dataLength;
        //add to list
        list = list
            +h_id
            +t_id
            +u_id
            +h_length
            +t_length
            +u_length;
    }

    console.log("sending data: "+list+'\n');
    socket.write(list+'\n');
}

function handleNewData(data){
    var dataString = data.toString();
    var charArray = dataString.split('');
    var charArrayIndex = 1;  //move past the I char

    if(charArray[0] == "I"){ //data instance
        //store to DB
        console.log("storing data instance to database");

        //get date and time for primary keys
        var timestamp = new Date();
        var date = timestamp.getFullYear()+"-"+(timestamp.getMonth()+1)+"-"+timestamp.getDate();
        var time = timestamp.toTimeString();

        //get client data, process and store
        for(var i=0; i<clients.length; i++){
            //console.log("extracting data for client: "+i);
            //extract data for this client and send to method for processing
            var clientData = new Array();
            var clientDataLength = clients[i][2] - 1; //take off for the Y/N char
            //console.log("clientDataLength = "+clientDataLength);
            for(var j=0; j<clientDataLength; j++){
                clientData[j] = charArray[charArrayIndex];
                charArrayIndex++;
            }
            //send client data to processing method
            clients[i][3](date, time, clientData);
        }


        /******************************
         //FOR SPARK - get last char for answer
         **********************************/
        //var clientData = new Array();
        //clientData[0] = charArray[charArrayIndex];
        //processEngineer(date, time, clientData);
    }
}

function processDemo(date, time, data){
    console.log("processing demographics data to store in DB");
    var age, comeFrom, cameWith, gender;
    if(data[0] != 'X'){
        age = data[0];
        console.log("age = "+age);
    }else{
        age = -1;
    }
    if(data[1] != 'X'){
        comeFrom = data[1];
        console.log("comeFrom = "+comeFrom);
    }else{
        comeFrom = -1;
    }
    if(data[2] != 'X'){
        cameWith = data[2];
        console.log("cameWith = "+cameWith);
    }else{
        cameWith = -1;
    }
    if(data[3] != 'X'){
        gender = data[3];
        console.log("gender = "+gender);
    }else{
        gender = -1;
    }
    var query = 'insert into demographics set ?';
    var storeset = {date: date, time: time, age: age, comeFrom: comeFrom, cameWith: cameWith, gender: gender};
    db.StoreToDB(dbName, query, storeset);
}

function processMood(date, time, data){
    console.log("processing mood data to store in DB");
    var slide1 = data[0];
    var slide2 = data[1];
    var slide3 = data[2];
    var slide4 = data[3];
    var slide5 = data[4];

    console.log("mood results = "+slide1+", "+slide2+", "+slide3+", "+slide4+", "+slide5);

    var query = 'insert into mood set ?';
    var storeset = {date: date, time: time, slide1: slide1, slide2: slide2, slide3: slide3, slide4: slide4, slide5: slide5};
    db.StoreToDB(dbName, query, storeset);
}

function processCrowd(date, time, data){
    console.log("processing crowd data to store in DB");

    var knob1 = data[0]+data[1];
    var knob2 = data[2]+data[3];
    var knob3 = data[4]+data[5];

    console.log("crowd results = "+knob1+", "+knob2+", "+knob3);

    var query = 'insert into crowd set ?';
    var storeset = {date: date, time: time, knob1: knob1, knob2: knob2, knob3: knob3};
    db.StoreToDB(dbName, query, storeset);
}

function processEvent(date, time, data){
    console.log("processing event data to store in DB");
    var spin1 = data[0];
    var spin2 = data[1];

    console.log("event results = "+spin1+", "+spin2);

    var query = 'insert into event set ?';
    var storeset = {date: date, time: time, spin1: spin1, spin2: spin2};
    db.StoreToDB(dbName, query, storeset);
}

function processPhone(date, time, data){
    console.log("processing phone data to store in DB");

    //store date and time
    var query = 'insert into phone set ?';
    var storeset = {date: date, time: time};
    db.StoreToDB(dbName, query, storeset);
}

/**
 * Constructor
 */
var VoxBox = function(){};

VoxBox.prototype.HandleInput = function(socket, data){

    if (data == "p"){
        handlePing();
    }else if(data == "L"){
        handleSlaveRequest(socket);
    }else{
        handleNewData(data);
    }
};

module.exports = VoxBox;
