
(function($){
	$.fn.extend({
		'initCalendar':function(obj){ //初始化日历\
			var warp = this;
			var date = new Date()
			var dateStr = this.getdate(date);

			warp.defalut.showData($('span.nowDate'),dateStr);
			
			warp.initDaysData(date);

			warp.on('click','tbody td',function(){
				warp.initDaysClick($(this));
			})
			
			warp.init_next_prev();
		},
		'init_next_prev':function(){  //上一月,下一月,上一年,下一年 按钮事件绑定
			var num = 0;
			var warp = $(this);
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
				warp.return_next_date(num);
				})
			})
		},
		'return_next_date':function(num){ // 月份选择后数据初始化
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
			this.find('span.nowDate').html(date.getFullYear() +'-'+ nextMonth +'-'+ day);
			this.initDaysData(date);
		},
		'getdate':function(date){   // 返回当前日期的字符串  0000-00-00;
			var year = date.getFullYear();
			var month = date.getMonth() + 1;
				month = month < 10 ? '0' + month : month;
			var day = date.getDate();
				day = day < 10 ? '0' + day : day;
			return year +'-'+ month +'-'+ day;  
		},
		'getMonthdata':function(date){  //返回当前月份的第一天为星期几和总共多少天
			var fir_Num = [];
			date.setDate(1);
			fir_Num.push(date.getDay());
			date.setMonth(date.getMonth() + 1);
			var lastDay = new Date(date - 3600000*24);
			fir_Num.push(lastDay.getDate());
			return fir_Num;
		},
		'initDaysData':function(date){ //当前月份的日期绑定到Dom中
			var warp = this;			
			var dd = this.find('span.nowDate').html().split('-');
			var thisMonthData = this.getMonthdata(date);
			warp.find('tbody td').html('').slice(thisMonthData[0]).each(function(index){
				index +=1;
				if(index <= thisMonthData[1]){
					$(this).html(index);
					if(index == dd[2]){ 
						$(this).addClass('nowDay');
					}
				}
			})
		},
		'initDaysClick':function(obj){ //日期下每天的Dom对象点击事件
			var warp = this;
			if(!obj.html()){
				return;
			}
			warp.find('tbody td').removeClass('nowDay');
			obj.addClass('nowDay');
			var nowDate = warp.getNewDay(obj.html());
			warp.defalut.showData(warp.find('span.nowDate'),nowDate);
		},
		'getNewDay':function(newDay){
			var warp = $(this);
			var oldDate = warp.find('span.nowDate').html().split('-');
			warp.parent('ul.option').hide();
			newDay = newDay < 10 ? '0'+newDay : newDay;
			return oldDate[0]+'-'+oldDate[1]+'-'+newDay;
		},
		'defalut':{      //显示日期。
			'showDataDom':{},
			/*'showData':function(str,num){
				var obj = this.showDataDom;
				if(num != null){
					obj[num].html(str);
					if(obj.length > 1){
						obj[num+1].html(str)
					}
				}else{
					for(var i in obj){
						obj[i].html(str);
					}
				}
			}*/
			'showData':function(obj,str){
				obj.html(str)
			}
		}
	});
})(jQuery);
$(function(){
	$('div.calendar').each(function(index){
		$(this).initCalendar();
	})
})