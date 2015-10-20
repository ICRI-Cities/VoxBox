/**
 * Created by sarahg on 19/10/15.
 */

var Database = require('./Database');
var db = new Database();
var dbName = "voxdb";

var queries = require('./Queries');

//refresh interval for getting MySQL queries into memory
var queryInterval = 30000;

//Original voxbox results
var demoResult = "[]";
var crowdResult = "[]";
var moodResult = "[]";
var eventResult = "[]";

module.exports.demoResult = demoResult;
module.exports.crowdResult = crowdResult;
module.exports.moodResult = moodResult;
module.exports.eventResult = eventResult;

//Spark festival results
var sparkDemoResult = "[]";
var sparkSliderResult = "[]";
var sparkDialResult = "[]";
var sparkRollerResult = "[]";
var sparkAdviceResult = "[]";

module.exports.sparkDemoResult = sparkDemoResult;
module.exports.sparkSliderResult = sparkSliderResult;
module.exports.sparkDialResult = sparkDialResult;
module.exports.sparkRollerResult = sparkRollerResult;
module.exports.sparkAdviceResult = sparkAdviceResult;

/****************************************************************
 LOADING VISUALISATION DATA INTO MEMORY
 ****************************************************************/
// loads data both in the startup and then reloads after queryInterval

//ORIGINAL
db.GetFromDB(dbName, queries.demoQuery, function(values) {
    module.exports.demoResult = JSON.stringify(values);
});
setInterval(function(){
    db.GetFromDB(dbName, queries.demoQuery, function(values) {
        module.exports.demoResult = JSON.stringify(values);
    });
}, queryInterval);

db.GetFromDB(dbName, queries.moodQuery, function(values) {
    module.exports.moodResult = JSON.stringify(values);
});
setInterval(function(){
    db.GetFromDB(dbName, queries.moodQuery, function(values) {
        module.exports.moodResult = JSON.stringify(values);
    });
}, queryInterval);

db.GetFromDB(dbName, queries.crowdQuery, function(values) {
    module.exports.crowdResult = JSON.stringify(values);
});
setInterval(function(){
    db.GetFromDB(dbName, queries.crowdQuery, function(values) {
        module.exports.crowdResult = JSON.stringify(values);
    });
}, queryInterval);

db.GetFromDB(dbName, queries.eventQuery, function(values) {
    module.exports.eventResult = JSON.stringify(values);
});
setInterval(function(){
    db.GetFromDB(dbName, queries.eventQuery, function(values) {
        module.exports.eventResult = JSON.stringify(values);
    });
}, queryInterval);


//SPARK FESTIVAL
db.GetFromDB(dbName, queries.sparkDemoQuery, function(values) {
    module.exports.sparkDemoResult = JSON.stringify(values);
});
setInterval(function(){
    db.GetFromDB(dbName, queries.sparkDemoQuery, function(values) {
        module.exports.sparkDemoResult = JSON.stringify(values);
    });
}, queryInterval);


db.GetFromDB(dbName, queries.sparkSliderQuery, function(values) {
    module.exports.sparkSliderResult = JSON.stringify(values);
});
setInterval(function(){
    db.GetFromDB(dbName, queries.sparkSliderQuery, function(values) {
        module.exports.sparkSliderResult = JSON.stringify(values);
    });
}, queryInterval);


db.GetFromDB(dbName, queries.sparkDialQuery, function(values) {
    module.exports.sparkDialResult = JSON.stringify(values);
});
setInterval(function(){
    db.GetFromDB(dbName, queries.sparkDialQuery, function(values) {
        module.exports.sparkDialResult = JSON.stringify(values);
    });
}, queryInterval);


db.GetFromDB(dbName, queries.sparkRollerQuery, function(values) {
    module.exports.sparkRollerResult = JSON.stringify(values);
});
setInterval(function(){
    db.GetFromDB(dbName, queries.sparkRollerQuery, function(values) {
        module.exports.sparkRollerResult = JSON.stringify(values);
    });
}, queryInterval);


db.GetFromDB(dbName, queries.sparkAdviceQuery, function(values) {
    module.exports.sparkAdviceResult = JSON.stringify(values);
});
setInterval(function(){
    db.GetFromDB(dbName, queries.sparkAdviceQuery, function(values) {
        module.exports.sparkAdviceResult = JSON.stringify(values);
    });
}, queryInterval);