/****
   Solar calendar:公历
   Julian：儒略历
   Jalaali：伊朗太阳历（波斯历）

*****/
function mod(a, b)
{
    return a - (b * Math.floor(a / b));
}

var Weekdays = new Array( "Sunday", "Monday", 
                          "Tuesday", "Wednesday",
                          "Thursday", "Friday", "Saturday" );
var JALAALI_WEEKDAYS = new Array(
    "شنبه", "یکشنبه","دوشنبه", 
    "سه شنبه","چهارشنبه", "پنج شنبه", "جمعه");
	
var JALAALI_EPOCH = 1948320.5;
var SOLAR_EPOCH = 1721425.5;

//闰年的判断						  
//  LEAP_SOLAR  --  Is a given year in the Gregorian calendar a leap year ?
function leap_solar(year)
{
    return ((year % 4) == 0) &&
            (!(((year % 100) == 0) && ((year % 400) != 0)));
}
//  LEAP_JALAALI  --  Is a given year a leap year in the Persian calendar ?
function leap_jalaali(year)
{
    return ((((((year - ((year > 0) ? 474 : 473)) % 2820) + 
                          474) + 38) * 682) % 2816) < 682;
}

function solar_to_julian(year, month, day)
{
    return (SOLAR_EPOCH - 1) +
           (365 * (year - 1)) +
           Math.floor((year - 1) / 4) +
           (-Math.floor((year - 1) / 100)) +
           Math.floor((year - 1) / 400) +
           Math.floor((((367 * month) - 362) / 12) +
           ((month <= 2) ? 0 :(leap_solar(year) ? -1 : -2)) + day);
}

//  JD_TO_JALAALI  --  Calculate Persian date from Julian day
function julian_to_jalaali(jd)
{
    var year, month, day, depoch, cycle, cyear, ycycle,
        aux1, aux2, yday;
    jd = Math.floor(jd) + 0.5;
    depoch = jd - jalaali_to_julian(475, 1, 1);
    cycle = Math.floor(depoch / 1029983);
    cyear = mod(depoch, 1029983);
    if (cyear == 1029982) {
        ycycle = 2820;
    } else {
        aux1 = Math.floor(cyear / 366);
        aux2 = mod(cyear, 366);
        ycycle = Math.floor(((2134 * aux1) + (2816 * aux2) + 2815) / 1028522) +
                    aux1 + 1;
    }
    year = ycycle + (2820 * cycle) + 474;
    if (year <= 0) {
        year--;
    }
    yday = (jd - jalaali_to_julian(year, 1, 1)) + 1;
    month = (yday <= 186) ? Math.ceil(yday / 31) : Math.ceil((yday - 6) / 30);
    day = (jd - jalaali_to_julian(year, month, 1)) + 1;
    return new Array(year, month, day);
}

//  JALAALI_TO_JD  --  Determine Julian day from Persian date
function jalaali_to_julian(year, month, day)
{
    var epbase, epyear;
    epbase = year - ((year >= 0) ? 474 : 473);
    epyear = 474 + mod(epbase, 2820);
    return day +((month <= 7) ? ((month - 1) * 31) :(((month - 1) * 30) + 6)) +
            Math.floor(((epyear * 682) - 110) / 2816) +
            (epyear - 1) * 365 +
            Math.floor(epbase / 2820) * 1029983 +
            (JALAALI_EPOCH - 1);
}
//  JD_TO_SOLAR  --  Calculate Gregorian calendar date from Julian day
function julian_to_solar(jd) {
    var wjd, depoch, quadricent, dqc, cent, dcent, quad, dquad,
        yindex, dyindex, year, yearday, leapadj;
    wjd = Math.floor(jd - 0.5) + 0.5;
    depoch = wjd - SOLAR_EPOCH;
    quadricent = Math.floor(depoch / 146097);
    dqc = mod(depoch, 146097);
    cent = Math.floor(dqc / 36524);
    dcent = mod(dqc, 36524);
    quad = Math.floor(dcent / 1461);
    dquad = mod(dcent, 1461);
    yindex = Math.floor(dquad / 365);
    year = (quadricent * 400) + (cent * 100) + (quad * 4) + yindex;
    if (!((cent == 4) || (yindex == 4))) {
        year++;
    }
    yearday = wjd - solar_to_julian(year, 1, 1);
    leapadj = ((wjd < solar_to_julian(year, 3, 1)) ? 0: (leap_solar(year) ? 1 : 2));
    month = Math.floor((((yearday + leapadj) * 12) + 373) / 367);
    day = (wjd - solar_to_julian(year, month, 1)) + 1;
    return new Array(year, month, day);
}

//输出的格式为0000-00-00
function getdate(dateArray){
	var year = dateArray[0];
	var month = dateArray[1];
		month = month < 10 ? '0' + month : month;
	var day = dateArray[2];
		day = day < 10 ? '0' + day : day;
	return year +'-'+ month +'-'+ day;  
}

//solar to jalaali
function solar_to_jalaali(date){
	var newDate = date.split('-');
	var year = newDate[0],month = newDate[1],day = newDate[2];
    var j;
	
    j = solar_to_julian(parseInt(year,10), parseInt(month,10), parseInt(day,10))+(Math.floor(0 + 60 * (0 + 60 * 0) + 0.5) / 86400.0);
	    
    perscal = julian_to_jalaali(j);
	
    return  getdate(perscal);
	}

//jalaali to solar
function jalaali_to_solar(date){
	
	var newDate = date.split('-');
	
	var year = newDate[0],month = newDate[1],day = newDate[2];
    var j,solar;
	
    j = jalaali_to_julian(parseInt(year,10), parseInt(month,10), parseInt(day,10));
 
    solar = julian_to_solar(j);
	
    return  getdate(solar);
	
	}