/**
 * Created by sarahg on 16/10/15.
 */

var httpPort = 5432;
var netPort = 4321;

var express= require('express');
var path = require('path');
var app = express();

var httpApp = require('http').Server(app);
var io = require('socket.io')(httpApp);

var netApp = require('net').Server();

var VoxBox = require('./VoxBox');
var vb = new VoxBox();

var VoxBoxViz = require('./VoxBoxViz');
var vbv = new VoxBoxViz();

app.use(express.static(path.join(__dirname, './public')));


//Handle client connections
io.on('connection', function(socket){
    vbv.HandleClient(socket);
});

/**
 * Webserver listener
 */
httpApp.listen(process.env.PORT || httpPort, function(){
    console.log("VoxBox http server listening on port: "+httpPort);
});



//Handle voxbox connections
netApp.on('connection', function(socket){
    console.log("VoxBox is connected...");

    socket.on('data', function (data) {
        console.log("GOT DATA");
        console.log("Got message from VoxBox: " + data.toString());
        vb.HandleInput(socket, data);
    });

    socket.on('close', function(){
        console.log("VoxBox connection closed");
    });

    socket.on('error', function(){
        console.log("error handled");
    });
});

/**
 * Net listener
 */
netApp.listen(netPort, function(){
    console.log("VoxBox net server listening on port: "+netPort);
});