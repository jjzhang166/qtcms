// JavaScript Document
;(function($){
	$.fn.Calendar = function(options){
		var root = this;
		 
		var defaults = { 
		     Jalaali:true, 
		    };
		
		var  obj = $.extend({},defaults,options);
       
		
		init_next_prev = function(){  //上一月,下一月,上一年,下一年 按钮事件绑定
			var num = 0;
			var warp = root;
			warp.find('div.calendar_top a').each(function(index){
				$(this).click(function(){
					warp.find('tbody td').removeClass('nowDay');
					switch(index){
						case 0:
							num  -= 12;
						break;
						case 1:
							num  -= 1;
						break;	
						case 2:
							num  += 1;
						break;
						case 3:
							num  += 12;
						break;	
					}
				return_next_date(num);
				})
			})
		}
		return_next_date = function(num){ // 月份选择后数据初始化
		//alert('月份选择后数据初始'+num);
		    var newdate;
		   if(obj.Jalaali){
				var date = getTodayPersian();
				var year = parseInt(num/12);
				var nextyear = date[0]+year;
				var month = num%12;
				var nextMonth = date[1]+ month;
				if(nextMonth>12){
					nextMonth = nextMonth%12;
					nextyear++;
				}
				   nextMonth =nextMonth < 10 ? '0' + nextMonth:nextMonth;
				var day = date[2];
					day = day<10?'0'+day:day;
				root.find('span.nowDate').html(nextyear+'-'+nextMonth+'-'+day);
				newdate = new Array(nextyear,nextMonth,1);
		   }else{
				var date = new Date();
				var month = num%12;
				var year = parseInt(num/12);
				date.setDate(1);
				date.setMonth(date.getMonth() + month);
				date.setYear(date.getFullYear() + year);
				var nextMonth = date.getMonth()+1;
					nextMonth = nextMonth < 10 ? '0'+ nextMonth : nextMonth;
				var day = date.getDate();
					day = day < 10 ? '0'+day : day ;
				root.find('span.nowDate').html(date.getFullYear() +'-'+ nextMonth +'-'+ day); 
				newdate = date;
			}
			
			initDaysData(newdate);
		}
		getdate0=function(date){   // 返回当前日期的字符串  0000-00-00;
			var year = date.getFullYear();
			var month = date.getMonth() + 1;
				month = month < 10 ? '0' + month : month;
			var day = date.getDate();
				day = day < 10 ? '0' + day : day;
			return year +'-'+ month +'-'+ day;  
		}
		getdate1 = function(date){   // 返回当前日期的字符串  0000-00-00;
			var year =date[0];
			var month = date[1];
				month = month < 10 ? '0' + month : month;
			var day = date[2];
				day = day < 10 ? '0' + day : day;
			return year +'-'+ month +'-'+ day;  
		}

		getMonthdata = function(date){  //返回当前月份的第一天为星期几和总共多少天
		   var fir_Num = [],weekday,lastDay;
		   
		   if(obj.Jalaali){
			  var y = date[0],m = date[1],d = date[2];
			  
			  var j =persian_to_jd(y,m,1);
				 weekday = parseInt(this.jwday(j),10);
				 weekday = weekday==6 ? 0 : weekday+1;
				  
			   lastDay = JalaaliMonthDay(y,m);
		   }else{
			  date.setDate(1); 
			  weekday = date.getDay();
			  date.setMonth(date.getMonth() + 1);
			  lastDay = new Date(date - 3600000*24).getDate();
		   }
			fir_Num.push(weekday);
			
			fir_Num.push(lastDay);
			
			return fir_Num;
		}
		
		initDaysData = function(date){ //当前月份的日期绑定到Dom中
			var warp = root;	
				
			var dd =warp.find('span.nowDate').html().split('-');
			var thisMonthData = getMonthdata(date);
			warp.find('tbody td').html('').slice(thisMonthData[0]).each(function(index){
				index +=1;
				if(index <= thisMonthData[1]){
					$(this).html(index);
					if(index == dd[2]){ 
						$(this).addClass('nowDay');
					}
				}
			})
		}
		initDaysClick = function(obj){ //日期下每天的Dom对象点击事件
			var warp = root;
			if(!obj.html()){
				return;
			}
			warp.find('tbody td').removeClass('nowDay');
			obj.addClass('nowDay');
			var nowDate = getNewDay(obj.html());
			showData(warp.find('span.nowDate'),nowDate);
		}
		getNewDay = function(newDay){
			var warp = root;
			var oldDate = warp.find('span.nowDate').html().split('-');
			//warp.parent('ul.option').hide();
			newDay = newDay < 10 ? '0'+newDay : newDay;
			return oldDate[0]+'-'+oldDate[1]+'-'+newDay;
		}
	    showData = function(obj,str){ //显示日期
				obj.html(str)
			}
		//  LEAP_GREGORIAN  --  Is a given year in the Gregorian calendar a leap year ?
		leap_gregorian = function(year)
		{
		  return ((year % 4) == 0) &&
				  (!(((year % 100) == 0) && ((year % 400) != 0)));
		}

		leapJalaali = function(year){  //判断是否是闰年
			
			 return ((((((year - ((year > 0) ? 474 : 473)) % 2820) + 474) + 38) * 682) % 2816) < 682;
			
			}
		JalaaliMonthDay = function(y,m){ //确定月份总共有多少天
			var month_day = 0;
			if(m > 0 && m <= 6){
			  month_day = 31;
			}else if(m >= 7 && m <= 11){
			  month_day = 30;
			}else if(m == 12){
				var bo = leapJalaali(y)? 1: 0;
				  month_day = 29+ bo;
			}
			return month_day;
        }
	    mod = function(a, b){
			 return a - (b * Math.floor(a / b));
			}
		jwday = function(j)
		{
			return mod(Math.floor((j + 1.5)), 7);
		}
		//  GREGORIAN_TO_JD  --  Determine Julian day number from Gregorian calendar date
		gregorian_to_jd = function(year, month, day)
		{    var GREGORIAN_EPOCH = 1721425.5;
		
			return (GREGORIAN_EPOCH - 1) +
				   (365 * (year - 1)) +
				   Math.floor((year - 1) / 4) +
				   (-Math.floor((year - 1) / 100)) +
				   Math.floor((year - 1) / 400) +
				   Math.floor((((367 * month) - 362) / 12) +
				   ((month <= 2) ? 0 :(leap_gregorian(year) ? -1 : -2)) + day);
		}
		//  JD_TO_PERSIAN  --  Calculate Persian date from Julian day
		  jd_to_persian = function(jd)
		  {
			  var year, month, day, depoch, cycle, cyear, ycycle,
				  aux1, aux2, yday;
			  jd = Math.floor(jd) + 0.5;
			  depoch = jd - persian_to_jd(475, 1, 1);
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
			  yday = (jd - persian_to_jd(year, 1, 1)) + 1;
			  month = (yday <= 186) ? Math.ceil(yday / 31) : Math.ceil((yday - 6) / 30);
			  day = (jd - persian_to_jd(year, month, 1)) + 1;
			  return new Array(year, month, day);
		  }
		  persian_to_jd = function(year, month, day)
		  {
			  var epbase, epyear;
			  var PERSIAN_EPOCH = 1948320.5;
			  epbase = year - ((year >= 0) ? 474 : 473);
			  epyear = 474 + mod(epbase, 2820);
			  return day +
					  ((month <= 7) ?
						  ((month - 1) * 31) :
						  (((month - 1) * 30) + 6)
					  ) +
					  Math.floor(((epyear * 682) - 110) / 2816) +
					  (epyear - 1) * 365 +
					  Math.floor(epbase / 2820) * 1029983 +
					  (PERSIAN_EPOCH - 1);
		  }
		  calcPersian = function(year,month,day)
		  {
			  var date,j;
		  
			  j = persian_to_jd(year,month,day);
			  date = jd_to_gregorian(j);
			  weekday = jwday(j);
			  return new Array(date[0], date[1], date[2],weekday);
		  }
		  //  calcGregorian  --  Perform calculation starting with a Gregorian date
		 calcGregorian = function(year,month,day)
		  {
			  month--;
			  var j, weekday;
			  
			  j = gregorian_to_jd(year, month + 1, day) +
					 (Math.floor(0 + 60 * (0 + 60 * 0) + 0.5) / 86400.0);
			 
			  perscal = jd_to_persian(j);
			  weekday = jwday(j);
			  return new Array(perscal[0], perscal[1], perscal[2],weekday);
		  }
		  getTodayGregorian = function()
		  {
			  var t = new Date();
			  var today = new Date();
			  var y = today.getYear();
			  if (y < 1000) {
				  y += 1900;
			  }
			  return new Array(y, today.getMonth() + 1, today.getDate(),t.getDay());
		  }
		  getTodayPersian = function()
		  {
			  var t = new Date();
			  var today = getTodayGregorian();
		  
			  var persian = calcGregorian(today[0],today[1],today[2]);
			  return new Array(persian[0],persian[1],persian[2]);
		  }
		
		return root.each(function(){
			 //初始化日历
			root.find('thead tr').hide().eq(obj.Jalaali).show();
				

			var date = obj.Jalaali ? getTodayPersian() : new Date();
			var dateStr =obj.Jalaali ? getdate1(date) : getdate0(date);
			
			showData($('span.nowDate'),dateStr);
			
			initDaysData(date);
            
			
			$(this).on('click','tbody td',function(){
				initDaysClick($(this));
			})
			
			init_next_prev();
			
			});
		}
})(jQuery);

$(document).ready(function() {
    //日历表的样式修改
		var bool = document.getElementById('commonLibrary').getLanguage() =='en_PR',
		  
            isJalaali = document.getElementById('commonLibrary').getIsPersian();
			
			if(bool&&isJalaali){
				
			  $('.calendar').Calendar({Jalaali:true});
			  styletb(1);
			  
			}else{
				
			   $('.calendar').Calendar({Jalaali:false});
			   styletb(6);
			   
		   }
});

//日历表的样式修改
   function styletb(num){
		$('.calendar').find('tbody tr td').removeClass('red').end()
			              .find('tbody tr').each(function(){
							       $(this).find('td').eq(num).addClass('red');
							   });
		} 
   
   