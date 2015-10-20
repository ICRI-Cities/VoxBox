/**
 * Created by sarahg on 19/10/15.
 */

var timeStamp = new Date();
//which day to show visualisations for
//(wildcards enabled, use * for all data from database)
//var dataDate = '\"'+ '2014-10-5' +'\"';
//or optionally uncomment the row below to use current date
var dataDate = '\"'+timeStamp.getFullYear()+"-"+(timeStamp.getMonth()+1)+"-"+timeStamp.getDate()+'\"';

//Original VoxBox queries
var demoQuery = 'select '+
    '((select count(age) from demographics where age < 3 and date= '+dataDate+') * 100 / count(d.age)) as age25, '+
    '((select count(gender) from demographics where gender = 0 and date = '+dataDate+') * 100 / count(d.gender)) as female, '+
    '((select count(cameWith) from demographics where cameWith = 3 and date = '+dataDate+') * 100 / count(d.cameWith)) as withFriends, '+
    '((select count(comeFrom) from demographics where comeFrom < 2 and date = '+dataDate+') * 100 / count(d.comeFrom)) as fromLondon '+
    'from demographics d where date = '+dataDate;
var moodQuery = 'select '+
    '((select count(slide5) from mood where slide5 > 4 and date = '+dataDate+') * 100 / count(m.slide5)) as safe, '+
    '((select count(slide1) from mood where slide1 < 5 and date = '+dataDate+') * 100 / count(m.slide1)) as bored, '+
    '((select count(slide4) from mood where slide4 > 4 and date = '+dataDate+') * 100 / count(m.slide4)) as inspired '+
    'from mood m where date = '+dataDate;
var crowdQuery = 'select '+
    '((select count(knob2) from crowd where knob2 < 8 and date = '+dataDate+') * 100 / count(c.knob2)) as notFitting, '+
    '((select count(knob1) from crowd where knob1 > 7 and date = '+dataDate+') * 100 / count(c.knob1)) as goodMood '+
    'from crowd c where date = '+dataDate;
var eventQuery = 'select '+
    '((select count(spin1) from event where (spin1 > 2 or spin1 = 0 ) and date = '+dataDate+' ) * 100 / count(e.spin1)) as goodExperience '+
    'from event e where date = '+dataDate;

module.exports.demoQuery = demoQuery;
module.exports.moodQuery = moodQuery;
module.exports.crowdQuery = crowdQuery;
module.exports.eventQuery = eventQuery;



//Spark festival queries
var sparkDemoQuery = 'select '+
    '((select count(gender) from demographics where gender = 0 and date = '+dataDate+' ) * 100 / count(d.gender)) as female '+
    'from demographics d where date = '+dataDate;
var sparkSliderQuery = 'select '+
    '((select sum(slide2) from mood where date = '+dataDate+') / count(m.slide2)) * 100 / 9 as thinking, '+
    '((select sum(slide1) from mood where date = '+dataDate+') / count(m.slide1)) * 100 / 9 as making '+
    'from mood m where date='+dataDate;
var sparkDialQuery = 'select '+
    '((select sum(knob2) from crowd where date = '+dataDate+') / count(c.knob2)) * 100 / 15 as practical, '+
    '((select sum(knob1) from crowd where date = '+dataDate+') / count(c.knob1)) * 100 / 15 as creative, '+
    '((select sum(knob3) from crowd where date = '+dataDate+') / count(c.knob3)) * 100 / 15 as logical '+
    'from crowd c where date='+dataDate;
var sparkRollerQuery = 'select '+
    '((select count(spin2) from event where spin2 = 0 and date = '+dataDate+') * 100 / count(e.spin2)) as computer, '+
    '((select count(spin1) from event where spin1 = 2 and date = '+dataDate+') * 100 / count(e.spin1)) as space, '+
    '((select count(spin1) from event where spin1 = 3 and date = '+dataDate+') * 100 / count(e.spin1)) as lab '+
    'from event e where date = '+dataDate;
var sparkAdviceQuery = 'select '+
    '((select count(type) from engineer where type = 0 and date = '+dataDate+' ) * 100 / count(eng.type)) as mechanical, '+
    '((select count(type) from engineer where type = 1 and date = '+dataDate+' ) * 100 / count(eng.type)) as electrical, '+
    '((select count(type) from engineer where type = 2 and date = '+dataDate+' ) * 100 / count(eng.type)) as aerospace, '+
    '((select count(type) from engineer where type = 3 and date = '+dataDate+' ) * 100 / count(eng.type)) as biochemical, '+
    '((select count(type) from engineer where type = 4 and date = '+dataDate+' ) * 100 / count(eng.type)) as software, '+
    '((select count(type) from engineer where type = 5 and date = '+dataDate+' ) * 100 / count(eng.type)) as civil '+
    'from engineer eng where date = '+dataDate;

module.exports.sparkDemoQuery = sparkDemoQuery;
module.exports.sparkSliderQuery = sparkSliderQuery;
module.exports.sparkDialQuery = sparkDialQuery;
module.exports.sparkRollerQuery = sparkRollerQuery;
module.exports.sparkAdviceQuery = sparkAdviceQuery;