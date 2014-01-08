
function addMouseStyle(obj,action){
	var width = obj.width();
	var left,top;
	obj.hover(function(){
		left = document.all ? parseInt(obj.css('backgroundPositionX')) : parseInt(obj.css('background-position').split('px')[0]);
		top = document.all ? parseInt(obj.css('backgroundPositionY')) : parseInt(obj.css('background-position').split('px')[1]);
		if(left != -width){
			obj.css('background-position',left-width+'px'+' '+top+'px');
		}
	},function(){
		obj.css('background-position',left+'px'+' '+top+'px');
	}).mousedown(function(){
		$(window).off();
		if(left == -width){
			obj.css('background-position',left-width+'px'+' '+top+'px');
		}else{
			obj.css('background-position',left-(2*width)+'px'+' '+top+'px');
		}
	}).mouseup(function(ev){
		if(action == 'toggle'){
			var H = obj.height();
			if(top == 0){
				obj.css('background-position',left-width+'px'+' '+(top-H)+'px');
			}else{
				obj.css('background-position',left-width+'px'+' 0px');
			}	
			top = parseInt(obj.css('backgroundPositionY')) || parseInt(obj.css('background-position').split('px')[1]);	
		}else if(action == 'hover'){
			obj.css('background-position',left-width+'px'+' '+top+'px');	
		}else{
			if(action == "switch"){
				var oSwitch = $('a.switch');

			}else{
				var ev = ev || window.event;
				var oSwitch = $('div .setViewNum');	
				oSwitch.each(function(index){
				var T = document.all ? parseInt($(this).css('backgroundPositionY')) : parseInt($(this).css('background-position').split('px')[1]);
				$(this).css('background-position','0px'+' '+T+'px').css('color','#B5B5B6');
				/*var oView = $('#playback_view');
				if(oView.length != 0){
					autoImages(oSwitch.index(ev.target)+1,oView);
				}*/
			})
				//alert(oSwitch.index(ev.target);
			}		
			obj.css('background-position',-width+'px'+' '+top+'px').css('color','#000');
			obj.parent('ul.option').prev('div.select').css('background-position',-width+'px'+' '+top+'px').css('color','#000');
			left=-width;
		}
		
	})
}

function ViewMax(type){
	var devListH = type == 'preview' ? 120 : 154 ;
	var WinW = $(window).width();
	var WinH = $(window).height();

	oView.css({
		width:WinW-oLeft.width()-8,
		height:WinH-oBottom.height()-110
	});
	if(oView.width()<680){
		oView.width(680);
	}
	if(oView.height()<450){
		oView.height(450);
	}
	oLeft.css({
		left:oView.width(),
		height:oView.height()+devListH
	});
	$('div.dev_list').height(oLeft.height()-288);
	oBottom.width(oView.width());
	$('#foot').css({
		top:oView.height()+212
	})
	if(type == 'preview'){
		$('#actionLog').width(oView.width()-6);
	}else{
		setTables();
	}	
}
function setTables(){ 
	$('table.table tr').each(function(index){
		var oTds = $(this).find('td');
		var W =  $('table.table').width() -50;
		oTds.width((W-80)/24);
		oTds.eq(0).width(80);		
	})
}
function set_drag(disX,X1,X2){
		var oDrag=$('div.play_time')
		$(document).mousemove(function(event){
				var left = event.pageX - disX;
			    left = left < X1 ? X1 : left;
				left = left > X2 ? X2 : left;
			oDrag.css('left',(left-2)+'px');	
		}).mouseup(function(){
			$(this).off();
		})
}
(function($){
	$.fn.extend({
		//dvr
		'toSwitch_0': function(){
			var warp = this;
			warp.find('li').each(function(index){
				$(this).click(function(){
					warp.find('li').removeClass('act');
					$(this).addClass('act');
					warp.nextAll('div.switch').hide();
					warp.nextAll('div.dvr_list').eq(index).show();
					switch(index)
					{
						case 0: dvr_devinfo_load_content();break;
						case 1: dvr_common_load_content();break;
						case 2: dvr_network_load_content();break;
						case 3: dvr_load('dvr_enc_chn_sel');break;
						case 4: dvr_load('dvr_record_chn_sel');break;
						case 5: dvr_load('dvr_screen_chn_sel');break;
						case 6: dvr_load('dvr_detect_chn_sel');break;
						case 7: dvr_load('dvr_ptz_chn_sel');break;
						case 8: dvr_load('dvr_alarm_chn_sel');break;
						default:break;
					}		
				})
			})
		},
		//ipc
		'toSwitch_1': function(){
			var warp = this;
			warp.find('li').each(function(index){
				$(this).click(function(){
					warp.find('li').removeClass('act');
					$(this).addClass('act');
					warp.nextAll('div.switch').hide();
					warp.nextAll('div.ipc_list').eq(index).show();
					switch(index)
					{
						case 0: devinfo_load_content(true);break;
						case 1: encode_load_content();break;
						case 2: network_load_content();break;
						case 3: user_management_load_content();break;
						case 4: time_load_content();break;
						//case 5: alert(5);break;
						default:break;
					}			
				})
			})
		},
		//client setting
		'toSwitch': function(){
			var warp = this;
			warp.find('li').each(function(index){
				$(this).click(function(){
					warp.find('li').removeClass('act');
					$(this).addClass('act');
					warp.nextAll('div.switch').hide().eq(index).show();	
				})
			})
		},
		'toSelect':function(){
			var This = this;
			var option = this.next('ul.option');
			this.click(function(){
				option.toggle(1,function(){ 
					if(option.is(':visible')){ 
						$(document).bind('click',function(e){
							if($(e.target).attr('class') == 'hover'){ 
								return false;
							}
							option.hide();	
							$(document).off();						
						})
					}
				});
			})
			option.find('a').click(function(){
				if($(this).attr('class') != 'hover'){
					This.next('ul.option').hide();
					This.find('span').html($(this).html());
					//This.find('#vendor_ID').val($(this).html());
					//This.find('input:hidden').val($(this).attr('key'));
				}
			})
		},
		'timeInput':function(options){
			var warp = $(this);
			var defaults = { 
			    Delimiter: ':',
			    initTime:'00 00 00',
			    timeFormat:24,
			    width:'20',
			    height:'18'
			}; 
			var opts = $.extend(defaults, options);
			var times = opts.initTime.split(' ');
			for(var i=0;i<3;i++){
				$('<input  maxlength="2"  value="'+times[i]+'" default="'+times[i]+'"/>').appendTo(warp);
				if(i<2){ 
					warp.html(warp.html()+opts.Delimiter);
				}
			}
			var inputs = warp.find('input');
			inputs.css({ 
				height:opts.height,
				width:opts.width,
				border:'none',
				background:warp.css('backgroundColor')
			})
			.each(function(index){ 
				$(this).focusout(function() {
					var str = availability($(this),index)
					if(str == '' || str.length <= 1){
						$(this).val($(this).attr('default'));
					}
				});

				$(this).focus(function(){
					$(this).val('');
				});

				$(this).keyup(function(){
					var str = availability($(this),index)
					if(str.length == 2){
						$(this).attr('default',$(this).val());
						inputs.eq(index + 1).focus();	
					}
				});	
			})
			function availability(obj,index){ 
				var str = obj.val().split('');
				if(index == 0){
					if(str[0] > 2){
						obj.val('2'+str[1]);	
					}	
					if(str[1] > 3){
						obj.val(str[0]+'3');
					}
				}else{
					if(str[0] > 6){
						obj.val('5'+str[1]);
					}	
				}
				return obj.val();
			}
		},
		'gettime':function (){
			if($(this).attr('class') == 'timeInput'){
				var time = []
				$(this).find('input').each(function(){ 
					time.push($(this).val());
				})
				return time.join(':');
			}
		},

		//数据填充部分
		'toCheck':function(){
			$(this).click(function(){
				return $(this).val($(this).prop('checked'));
			})
		},
		'dataIntoVal':function (val){ 
			return $(this).val(val);
		},
		'dataIntoHtml':function(val){ 
			return $(this).html(val);
		},
		'dataIntoSelected':function (val){
			if(typeof val == 'string'){
				if($(this).val() == val){
					$(this).prop('checked',true);
				}
			}else{
				$(this).prop('checked',Boolean(val)).val(val);
			}
			return $(this);
		}
	})
})(jQuery)
$(function(){

	$('div.select').each(function(){
		$(this).toSelect()
	});

	$('#top div.top_nav li').mousedown(function(){ 
		$(this).css('background-position','0 -52px');
	}).mouseup(function(){ 
			$(this).css('background-position','0 0');
	})
	
	$('div.timeInput').each(function(index){ 	
		if(index%2 ==0){ 
			$(this).timeInput();
		}else{
			$(this).timeInput({'initTime':'23 59 59'});
		}
	})
})
// 辅助方法.
function del(str) {   //数组去除重复
	var a = {}, c = [], l = str.length; 
	for (var i = 0; i < l; i++) { 
	var b = str[i]; 
	var d = (typeof b) + b; 
	if (a[d] === undefined){ 
		c.push(b); 
		a[d] = 1; 
		} 
	} 
	return c; 
}

function sortNumber(a,b){ //数组升序排列
	return a-b;
}

function firstUp(str){  //字符串首字母大写
	var a = str.split('');
	a[0] = a[0].toUpperCase();
	return a.join('');
}
function show(data){
	var index='default'
	var str = 'Null'
	$('#test').html('');
	if(typeof(data) != 'string'){
		for(i in data){ 
			index = i;
			str = data[i];
			$('<span>'+index+'</span>:<span>'+str+'/</span>').appendTo($('#test'));
		}
	}else{
		$('<span>'+index+'</span>:<span>'+data+'/</span>').appendTo($('#test'));
	}
}
function addZero(num){   //数字小于0的时候用0补一位.
	num = num.toString();
	return num = num<10 ? '0'+num : num;
}
function showdata(id,type){ 
	var submit = $('#'+type).find('.confirm:visible').attr('id');
	var str =submit+'/'+id +'/';
	$('#'+type).find('[id]').each(function(){ 
		str += $(this).attr('id')+':'+$(this).val()+'/';
	})
	show(str);
}
//弹出框部分操作
function closeMenu(){
	$('#iframe,div.confirm, div.menu').hide();
	$('#menusList div.menu input.data').remove();
	$('#menusList div.menu input:text').val('');
	$('#confirm h4').html('');
	$('div.menue').each(function(){ 
		$(this).find('div.close:last').html('取消');
	})
	
}
function Confirm(str){
	$('#confirm h4').append('<p>'+str+'</p>');
	objShowCenter($('#confirm'));
}
function objShowCenter(obj){ //调整弹出框定位 居中
	$('#iframe').hide().show();
	obj.css({
		top:($(window).height() - obj.height())/2,
		left:($(window).width() - obj.width())/2
	}).show();
}
function returnTime(sInt){ 
	var H = sInt/3600;
		H = addZero(parseInt(H));
	var M = (sInt-H*3600)/60
		M = addZero(parseInt(M));
	var S = sInt-H*3600-M*60;
		S = addZero(parseInt(S));
	/*if(sHours[1]){
		var sM = sHours[1].slice(0,2),
    	M = addZero(parseInt(sM*0.6));
    }else{ 
    	M = '00';
    }*/
	/*var type = parseInt($('#type span').attr('type'))
		type = type == 0 ? 15 : 1 << type;
	var date = $("div.calendar span.nowDate").html();
	var begin = date
	var end = date*/
	return H+':'+M+':'+S;
}
function getType(o) { 
var _t; return ((_t = typeof(o)) == "object" ? o==null && "null" || Object.prototype.toString.call(o).slice(8,-1):_t).toLowerCase(); 
} 
function toJsArray(obj){ 
	var a=[];
	for(i in obj){ 
		a.push(obj[i]);
	}
	return a;
}
function gettime(objs){
	var time = []
	objs.each(function(){ 
		time.push($(this).val());
	})
	return time.join(':');
}
function time2Sec(str){ 
	var a =str.split(':');
	return parseInt(a[0])*60*60+parseInt(a[1])*60+parseInt(a[2]);
}