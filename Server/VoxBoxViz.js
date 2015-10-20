/**
 * Created by sarahg on 19/10/15.
 */

var results = require('./Results');

//refresh interval for sending socket data to clients
//(shouldn't be too long as new page is empty until data arrives)
var socketInterval = 60000;

//console.log("demoResults = "+results.demoResult);

var VoxBoxViz = function(){};

VoxBoxViz.prototype.HandleClient = function(socket){
    //ORIGINAL
    socket.emit('demoResults', results.demoResult.substr(1,results.demoResult.length-2));
    socket.emit('moodResults', results.moodResult.substr(1,results.moodResult.length-2));
    socket.emit('crowdResults', results.crowdResult.substr(1,results.crowdResult.length-2));
    socket.emit('eventResults', results.eventResult.substr(1,results.eventResult.length-2));

    setInterval(function(){
        socket.emit('demoResults', results.demoResult.substr(1,results.demoResult.length-2));
    }, socketInterval);

    setInterval(function(){
        socket.emit('moodResults', results.moodResult.substr(1,results.moodResult.length-2));
    }, socketInterval);

    setInterval(function(){
        socket.emit('crowdResults', results.crowdResult.substr(1,results.crowdResult.length-2));
    }, socketInterval);

    setInterval(function(){
        socket.emit('eventResults', results.eventResult.substr(1,results.eventResult.length-2));
    }, socketInterval);


    //SPARK FESTIVAL
    socket.emit('sparkDemoResults', results.sparkDemoResult.substr(1,results.sparkDemoResult.length-2));
    socket.emit('sparkSliderResults', results.sparkSliderResult.substr(1,results.sparkSliderResult.length-2));
    socket.emit('sparkDialResults', results.sparkDialResult.substr(1,results.sparkDialResult.length-2));
    socket.emit('sparkRollerResults', results.sparkRollerResult.substr(1,results.sparkRollerResult.length-2));
    socket.emit('sparkAdviceResults', results.sparkAdviceResult.substr(1,results.sparkAdviceResult.length-2));

    setInterval(function(){
        socket.emit('sparkDemoResults', results.sparkDemoResult.substr(1,results.sparkDemoResult.length-2));
    }, socketInterval);

    setInterval(function(){
        socket.emit('sparkSliderResults', results.sparkSliderResult.substr(1,results.sparkSliderResult.length-2));
    }, socketInterval);

    setInterval(function(){
        socket.emit('sparkDialResults', results.sparkDialResult.substr(1,results.sparkDialResult.length-2));
    }, socketInterval);

    setInterval(function(){
        socket.emit('sparkRollerResults', results.sparkRollerResult.substr(1,results.sparkRollerResult.length-2));
    }, socketInterval);

    setInterval(function(){
        socket.emit('sparkAdviceResults', results.sparkAdviceResult.substr(1,results.sparkAdviceResult.length-2));
    }, socketInterval);

};

module.exports = VoxBoxViz;