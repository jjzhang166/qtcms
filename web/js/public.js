/*

   该文件不要涉及到设备/插件/播放器的数据,及相关逻辑操作处理。
	1.UI上的公共事件添加 及响应处理.
	2.辅助方法.
	3.其他数据相关事件响应处理和个别形式的事件, 请在对应JS文件中处理
	4.如UI上有相同的逻辑操作, 请慎重在文件添加.

*/
function addMouseStyle(obj,action){  //按钮UI响应
	var width = obj.width();
	var left,top;
	obj.hover(function(){
		left = document.all ? parseInt(obj.css('backgroundPositionX'),10) : parseInt(obj.css('background-position').split('px')[0],10);
		top = document.all ? parseInt(obj.css('backgroundPositionY'),10) : parseInt(obj.css('background-position').split('px')[1],10);
		if(left != -width){
			obj.css('background-position',left-width+'px'+' '+top+'px');
		}
	},function(){
		obj.css('background-position',left+'px'+' '+top+'px');
	}).mousedown(function(){
		//$(window).off();
		if(left == -width){
			obj.css('background-position',left-width+'px'+' '+top+'px');
		}else{
			obj.css('background-position',left-(2*width)+'px'+' '+top+'px');
		}
	}).mouseup(function(ev){
		if(action == 'toggle'){
			var H = obj.height();
			var a = obj.attr('toggle');
			if(a){
				obj.css('background-position',left-width+'px'+' '+(top+H)+'px');
				obj.removeAttr('toggle');
			}else{
				obj.attr('toggle',1);
				obj.css('background-position',left-width+'px'+' '+(top-H)+'px');
			}	
			top = parseInt(obj.css('backgroundPositionY'),10) || parseInt(obj.css('background-position').split('px')[1],10);	
		}else if(action == 'hover0'){
			obj.css('background-position',left-width+'px'+' '+top+'px');	
		}else{
			if(action == "switch"){
				var oSwitch = $('a.switch');
			}else{
				var ev = ev || window.event;
				var oSwitch = $('div .setViewNum');	
				oSwitch.each(function(index){
				var T = document.all ? parseInt($(this).css('backgroundPositionY'),10) : parseInt($(this).css('background-position').split('px')[1],10);
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
function addMouseStyleByLimit(obj,action,maincode){  //按钮UI响应
	var width = obj.width();
	var left,top,initLeft,initTop;
	obj.hover(function(){
		left = document.all ? parseInt(obj.css('backgroundPositionX'),10) : parseInt(obj.css('background-position').split('px')[0],10);
		top = document.all ? parseInt(obj.css('backgroundPositionY'),10) : parseInt(obj.css('background-position').split('px')[1],10);
		initLeft=left;
		initTop=top;
		if(left != -width){
			obj.css('background-position',left-width+'px'+' '+top+'px');
		}
	},function(){
		obj.css('background-position',left+'px'+' '+top+'px');
	}).mousedown(function(){
		//$(window).off();
		if(left == -width){
			obj.css('background-position',left-width+'px'+' '+top+'px');
		}else{
			obj.css('background-position',left-(2*width)+'px'+' '+top+'px');
		}
	}).mouseup(function(ev){
		
		if(action == 'toggle'){
			var H = obj.height();
		    var a = obj.attr('toggle');
			var itema = document.getElementById('atuoSearchDevice').checkUserLimit(maincode.toString(2),0);
			if(itema==0){
				  
				  if(a){
					  obj.css('background-position',left+'px'+' '+(top+H)+'px');
				  }else{
					  obj.css('background-position',left+'px'+' '+(top-H)+'px');
				  }	
			}else if(itema==1){
				var show = autoSearchDev.showUserLoginUi(336,300);
				if(show==0){
				  if(a){
					  obj.css('background-position',left+'px'+' '+(top+H)+'px');
				  }else{
					  obj.css('background-position',left+'px'+' '+(top-H)+'px');
				  }	
				}else{
					
					obj.css('background-position',initLeft+'px'+' '+initTop+'px');
					
				}
			
			}else{
				 obj.css('background-position',initLeft+'px'+' '+initTop+'px');
				 
			}
		   if(a){
			   obj.removeAttr('toggle'); 
		   }else{
			   obj.attr('toggle',1);
		   }
			top = parseInt(obj.css('backgroundPositionY'),10) || parseInt(obj.css('background-position').split('px')[1],10);	
		}else if(action == 'hover'){
			var itema = document.getElementById('atuoSearchDevice').checkUserLimit(maincode.toString(2),0);
			if(itema==1){
				 autoSearchDev.showUserLoginUi(336,300);
			}
			obj.css('background-position',left+'px'+' '+top+'px');
		}else{
			if(action == "switch"){
				var oSwitch = $('a.switch');
			}else{
				var ev = ev || window.event;
				var oSwitch = $('div .setViewNum');	
				oSwitch.each(function(index){
				var T = document.all ? parseInt($(this).css('backgroundPositionY'),10) : parseInt($(this).css('background-position').split('px')[1],10);
				$(this).css('background-position','0px'+' '+T+'px').css('color','#B5B5B6');
			})
			}
			
			var itema = document.getElementById('atuoSearchDevice').checkUserLimit(maincode.toString(2),0);
			if(itema==0){
				 obj.css('background-position',-width+'px'+' '+top+'px').css('color','#000'); 
			    obj.parent('ul.option').prev('div.select').css('background-position',-width+'px'+' '+top+'px').css('color','#000');
			    left=-width;
				
			}else if(itema==1){
				 var show = autoSearchDev.showUserLoginUi(336,300);
				 if(show==0){
					obj.css('background-position',-width+'px'+' '+top+'px').css('color','#000'); 
			        obj.parent('ul.option').prev('div.select').css('background-position',-width+'px'+' '+top+'px').css('color','#000');
			        left=-width;
					 
				 }else{
				    var pos = obj.parent('ul.option').prev('div.select').css('background-position').split('px');
					 obj.parent('ul.option').find('li').each(function(){
						 if(parseInt($(this).css('background-position').split('px')[1],10)==parseInt(pos[1],10)){
							 $(this).css('background-position',pos[0]+'px '+pos[1]+'px');
						 }
					});
				 }
				 
		  }else{
			 var pos = obj.parent('ul.option').prev('div.select').css('background-position').split('px');
					 obj.parent('ul.option').find('li').each(function(){
						 if(parseInt($(this).css('background-position').split('px')[1],10)==parseInt(pos[1],10)){
							 $(this).css('background-position',pos[0]+'px '+pos[1]+'px');
						 }
					});
		  }
			
		}		
	})
}
function set_drag(X1,X2,oDrag){  // 回放页面的拖拽条
	var oNow=$('#now_time');	
	var b=oDrag.hasClass('now_sound'),
		left;
	if(b){
		var veiwObj = getAudioObj();
		var oWarpLeft = $('#sound');
	}

	$(document).mousemove(function(event){
			left = event.pageX;
		    left = left < X1 ? X1 : left;
			left = left > X2 ? X2 : left;
		if(b){
			left=left-oWarpLeft.offset().left;
			oWarpLeft.find('p:last').width(left);
			veiwObj.SetVolume(left);
		}else{
			showNowPlayBackTime(oNow,left-X1,X2-X1);
		}
		oDrag.css('left',left-1.5);
	}).mouseup(function(){
		$(this).off();
	})
}
function showNowPlayBackTime(oNow,oleft,X2){
	//return;
	oNow.html(returnTime((oleft/X2)*24*3600));
}
(function($){   // 
	$.fn.extend({
		//client setting
		'toSwitch': function(){
			var warp = $(this);
			var sClass = warp.find('li:first').attr('class');
			var num = warp.find('li').length;
			warp.find('li').each(function(index){
				$(this).on('click',function(){
					warp.show().nextAll(':lt('+num+')').show();
					warp.find('li').removeClass(sClass);
					$(this).addClass(sClass);
					var showDiv = warp.nextAll(':lt('+num+')').hide().eq(index).show();
				})
			})
		},
		'toSelect':function(){ //模拟HTML下拉选择框 JQ插件形式
			var This = this.find('input').prop('disabled',true).end();
			var option = This.next('ul.option').find('input').prop('disabled',true).end();
			This.click(function(event){
				event.stopPropagation();
				if(option.is(':hidden')){ 
					$('ul.option').hide();
					option.show();
					$(document).bind('click',function(e){
						/*if($(e.target).attr('class') == 'hover'){ 
							return false;
						}*/
						option.hide();	
						$(document).off();						
					})
				}else{
					$('ul.option').hide();
				}
			})
			option.on('click','li',function(){
				//if($(this).attr('class') != 'hover'){
					var str = $(this).find('input').val();
					//str = str.match(/<\/?\w+>/g) ? str.match(/[\u4e00-\u9fa5]+/g)[0] : str;
					str = str.match(/<\/?\w+>/g) ? str.match(/[\u4e00-\u9fa5]+/g) : str;
					$('ul.option').hide();
					var data = $(this).find('input').attr('data');
					This.find('input[data]').val(str).attr('data',data);
					/*if(This.next('ul[action]')){
						This.find('#'+This.next('ul[action]').attr('action')).val(value);
					}*/
					option.find('input:hidden[id]').dataIntoVal(data);
				//}
			})

			/*1
				设备设置重构IPC完成阶段.  兼容NVR/DVR 设置菜单的下拉框。

				NVR/DVR重构完成后 请删除。
			*/
			option.on('click','a',function(){
				//if($(this).attr('class') != 'hover'){
					var str = $(this).html();
					str = str.match(/<\/?\w+>/g) ? str.match(/[\u4e00-\u9fa5]+/g)[0] : str;
					var value = $(this).attr('value')
					$('ul.option').hide();
					This.find('span').html(str).attr('value',value);
					if(This.next('ul[action]')){
						This.find('#'+This.next('ul[action]').attr('action')).val(value);
					}
					//This.find('input:hidden').val($(this).attr('value'));
				//}
			})
			/*1*/
		},

		'timeInput':function(options){  //时间输入框
			$(this).html('');
			var warp = $(this);
			var defaults = { 
			    Delimiter: ':',
			    initTime:'00:00:00',
			    timeFormat:24,
			    width:'20',
			    height:'18'
			}; 
			var opts = $.extend(defaults, options);
			var times = opts.initTime.split(':');
			for(var i=0;i<3;i++){
				$('<input type="text" maxlength="2"  value="'+times[i]+'" default="'+times[i]+'"/>').appendTo(warp);
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
					var str = $(this).val();
					$(this).attr('default',availability($(this),index));
					if(str.length == 1){
						$(this).val('0'+str).attr('default','0'+str);
					}
					
				});

				$(this).focus(function(){
					$(this).val('');
				});

				$(this).keyup(function(){
					if($(this).val().length > 1){
						$(this).attr('default',availability($(this),index));
						inputs.eq(index + 1).focus();	
					}
				});	
			})
			function availability(obj,index){   //调整输入的时间范围
				var str = obj.val();
				if(!/\d{1,2}/.test(str)){
					obj.val(obj.attr('default'));
				}else{
					if(index == 0){
						str > 23 &&	obj.val('23');
					}else{
						str > 59 &&	obj.val('59');	
					}
				}
				return obj.val();
			}
		},

		'gettime':function (){  // 初始化好的时间控件    的相应获时间件方法。
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
			return $(this).val(val).attr('data',val);
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
		},
		// 表单tr点击同步首个td下的checkbox点击
		'SynchekboxClick':function(){
			var warp = $(this);
			var warpId = warp.attr('id');
			var oSelectAll=$('#'+warpId+'_SelectAll');
								
			if(warpId == 'SerachedDevList' || warpId == 'search_resultFile'){
				warp.on('click','tr',function(){
					$(this).find('input:checkbox').click();
				})
			}

			warp.on('click',':checkbox',function(event){
				var b = true;
				warp.find(':checkbox:enabled').each(function(){
					if(!$(this).prop('checked')){
						b = false;
					}
				})

				oSelectAll.prop('checked',b);
				event.stopPropagation();
			})

			oSelectAll.click(function(){
				warp.find(':checkbox:enabled').prop('checked',$(this).is(':checked'));
			})

			return warp

			/*$('#tableSelectAll').on('click',function(){  // 全选
				if($(this).attr('status')){
					warp.find('input:checkbox').not(':checked').prop('checked',true);
					$(this).removeAttr('status');
				}else{
					warp.find('input:checkbox').prop('checked',false);
					$(this).attr('status',1);
				}
				
			})*/

			/*$('#tableAntiElection').on('click',function(){ //全不选
				warp.find('input:checkbox').prop('checked',false);
			})*/
		}
	})
})(jQuery)

$(function(){
	/*console.log = function(){
		return;
	}*/
	// 导航页面跳转
	var url =['index.html','play_back.html','super.html','backup.html','device.html','','log.html'];
	$('div.top_nav li').each(function(index){
		$(this).on('click',function(){
			if(index==5){
				lock(); 
				return;
			}
			var src = location.href.replace(/.*?(\w+\.html)/g,'$1'),
				srcAct = '';
				dstAct = 'new';
			var dst = url[index];
			if(dst == src){
				return;
			}
			if(dst == 'index.html'){ 
				srcAct = 'index';
				dstAct = 'reload';
			}
			src = '/skins/default/'+src;
			dst = '/skins/default/'+dst;

			window.status = '<pageaction SrcUrl="'+src+'" SrcAct="'+srcAct+'" DstUrl="'+dst+'" DstAct="'+dstAct+'"></pageaction>';
		})
	})
	//下拉菜单模拟	
	$('div.select').each(function(){
		$(this).toSelect()
	});
	//导航UI切换
	$('#top div.top_nav li').mousedown(function(){ 
		$(this).css('background-position','0 -52px');
	}).mouseup(function(){
			$(this).css('background-position','0 0');
	})
	//时间空间初始化
	$('div.timeInput').each(function(index){ 	
		if(index%2 ==0){ 
			$(this).timeInput();
		}else{
			$(this).timeInput({'initTime':'23:59:59'});
		}
	})
	//带表弟的表单切换控件.
	$('ul.switchlist,.ope_list').each(function(){
		$(this).toSwitch();
	})
	
	document.oncontextmenu = function(e){  //文档默认右键事件冒泡取消
		var e = e || window.event;
		if(e.target.tagName != 'BODY'){
			return;
		}
	}

	$('.hover0').each(function(){  // 按钮元素添加鼠标事件对应样式
		var action = $(this).attr('class').split(' ')[0];
		addMouseStyle($(this),action);
		
	})
})///
function triggerOnclick(id,sEv){ 
	try{
        //非IE
        var fireOnThis = document.getElementById(sEv);
        var evObj = document.createEvent('HTMLEvents');
        evObj.initEvent(sEv, true, false);
        fireOnThis.dispatchEvent(evObj);
    }
    catch(e)
    {
        //IE
        document.getElementById(id).fireEvent(sEv);
    }
}
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

function addZero(num){   //数字小于0的时候用0补一位.
	num = num.toString();
	return num = num<10 ? '0'+num : num;
}

//弹出框部分操作
function closeMenu(){ 
	$('#iframe,div.confirm,div.menu').hide();
	$('#confirm').find('h4,span').html('');
	$('#menusList div.menu').find('input.data').remove().end().find(':text,:password').not('[default]').val('');
		/*.end().end().each(function(){
			$(this).find('div.close:last').html(lang.Cancel);	
		})*/
	/*$('#menusList div.menu input.data').remove();
	$('#menusList div.menu input:text').val('');
	$('div.menu').each(function(){ 
		$(this).find('div.close:last').html(lang.Cancel);
	})*/
}
function Confirm(str,b,fn){
	var obj = $('#confirm');
	if(b){
		var oVisible = $('#menusList div.menu:visible').not('#confirm').css('z-index','0');
		obj.find('.close').off('click').click(function(){
			oVisible.css('z-index','1000');
			$('#confirm').find('h4,span').html('').end().hide();
		})
	}else{
		obj.find('.close').click(function(){
			closeMenu();
			typeof(fn) == 'function' && fn();
		})
	}
	obj.find('h4').append('<p>'+str+'</p>');
	objShowCenter($('#confirm'));
}
function confirm_tip(str){
	var obj = $('#confirm_tips');
	obj.find('span').html(_T('tip'));
	obj.find('h4').html('<p>'+str+'</p>');
	objShowCenter($('#confirm_tips'));
}
function objShowCenter(obj){ //调整弹出框定位 居中
	$('#iframe').hide().show();
	obj.css({
		top:($(window).height() - obj.height())/2,
		left:($(window).width() - obj.width())/2,
		zIndex:'1000'
	}).show();
	if(obj.attr('id') == 'confirm'){
		if(obj.find('div.confirm:visible').length == 0){
			obj.find('div.close').html(_T('Confirm'));
		}else{
			obj.find('div.close').html(_T('Cancel'));
		}
	}
}


function returnTime(sInt){  //秒转换时间.
	var H = parseInt(sInt,10)/3600;
		H = addZero(parseInt(H,10));
	var M = (sInt-H*3600)/60
		M = addZero(parseInt(M,10));
	var S = sInt-H*3600-M*60;
		S = addZero(parseInt(S,10));
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
function time2Sec(str){  //把时间转换成秒
	var a =str.split(':');
	return parseInt(a[0],10)*60*60+parseInt(a[1],10)*60+parseInt(a[2],10);
}
 
function checkHasObj(oSil,obj){
	var b = 0;			
	oSil.each(function(){ddd
		if($(this).is(obj)){
			return b=1;
		}
	})
	return b;
}


function T(){//页面文本切换输出
	var str = lang ? lang[arguments[0]] : arguments[0].replace('_',' ');
	if(arguments.length ==1){
		document.write(str);
	}else{
		str = hintTrans(arguments);
	};
	return str;
}

function _T(str){
	return lang[str] ? lang[str] : str;
}

function hintTrans(obj){ //操作提示语言转换.
	var str=lang[obj[0]];
	for(var i=1;i<obj.length;i++){
		str = str.replace(/_V_/,obj[i]);
	}
	return str;		
}

function getAudioObj(){   //返回当前页面播放的插件对象。
	var oAudioObj = {};
	if($('#previewWindows')[0]){
		oAudioObj = $('#previewWindows')[0];
	}else{
		if(bool){
			oAudioObj = $('#playbackLocl')[0];
		}else{
			oAudioObj = $('#playback')[0];	
		}
	}
	return oAudioObj;
}

function theadtbody(t1,t2){
	t1.each(function(index){
		t2.eq(index).width($(this).width());
	})
}

//input  值翻译
function _t(obj){
	obj.each(function(){
		var val = $(this).val(),
			str = lang[val] || val;
		if(val.indexOf('Screen') != -1){
			var t = val.split(' ');
			str = lang[t[1]] ? t[0]+lang[t[1]] : str;
		}
		$(this).val(str);
	})
}

function renewtime(){
	var myDate = new Date,

	yy=myDate.getFullYear(),

	mm=addZero(parseInt(myDate.getMonth(),10)+1),

	dd=addZero(myDate.getDate()),

	hh=addZero(myDate.getHours()),

	mi=addZero(myDate.getMinutes()),

	ss=addZero(myDate.getSeconds());

	return yy + "-" + mm + "-" + dd + "  " + hh + ":" + mi + ":" + ss;
}

function lock(){
	  autoSearchDev.showUserLoginUi(336,300); 
}

