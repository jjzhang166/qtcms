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
		left = document.all ? parseInt(obj.css('backgroundPositionX')) : parseInt(obj.css('background-position').split('px')[0]);
		top = document.all ? parseInt(obj.css('backgroundPositionY')) : parseInt(obj.css('background-position').split('px')[1]);
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

function setTables(){   // 回放页面底部表格最大化相应调整
	$('table.table tr').each(function(index){
		var oTds = $(this).find('td');
		var W =  $('table.table').width()-120;

		oTds.width((W)/24);
		oTds.eq(0).width(80);
	})
}
function set_drag(X1,X2,oDrag){  // 回放页面的拖拽条
	/*var oNow=$('#now_time');
	showNowPlayBackTime(oNow,oDrag.offset().left,X2);*/
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
			if(veiwObj.id == 'playback'){
				document.getElementById('playbackLocl').SetVolume(left);
			}else if(veiwObj.id == 'playbackLocl'){
				document.getElementById('playback').SetVolume(left);
			}
			//veiwObj.vol = left;
		}/*else{
			showNowPlayBackTime(oNow,left,X2);
		}*/
		oDrag.css('left',left-1);
	}).mouseup(function(){
		$(this).off();
	})
}
function showNowPlayBackTime(oNow,oleft,X2){
	return;
	oNow.html(returnTime((oleft-81)/(X2-81)*24*3600));
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
					warp.nextAll(':lt('+num+')').hide().eq(index).show();	
				})
			})
		},
		'toSelect':function(){ //模拟HTML下拉选择框 JQ插件形式
			var This = this;
			var option = This.next('ul.option');
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
			option.on('click','a',function(){
				//if($(this).attr('class') != 'hover'){
					var str = $(this).html();
					str = str.match(/<\/?\w+>/g) ? str.match(/[\u4e00-\u9fa5]+/g)[0] : str;
					console.log(str);
					$('ul.option').hide();
					This.find('span').html(str).attr('value',$(this).attr('value'));
					if(This.find('ul[action]')){
						This.find('#'+This.find('ul[action]').attr('action')).val($(this).attr('value'));
					}
					This.find('input:hidden').val($(this).attr('value'));
				//}
			})
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
			function availability(obj,index){   //调整输入的时间范围
				var str = obj.val().split('');
				if(index == 0){
					if(str[0] > 2){
						obj.val('2');	
					}	
					if(str[1] > 3){
						obj.val('23');
					}
				}else{
					if(str[0] > 5){
						obj.val('5');
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
		},
		// 表单tr点击同步首个td下的checkbox点击
		'SynchekboxClick':function(){
			var warp = $(this);
			if(warp.length == 0 || warp[0].nodeName != 'TBODY'){
				return;
			}

			warp.on('click','tr',function(){
				$(this).find('input:checkbox').click();
			})

			warp.on('click',':checkbox',function(event){
				event.stopPropagation();
			})

			$('#tableSelectAll').on('click',function(){  // 全选
				if($(this).attr('status')){
					warp.find('input:checkbox').not(':checked').prop('checked',true);
					$(this).removeAttr('status');
				}else{
					warp.find('input:checkbox').prop('checked',false);
					$(this).attr('status',1);
				}
				
			})

			/*$('#tableAntiElection').on('click',function(){ //全不选
				warp.find('input:checkbox').prop('checked',false);
			})*/
		}
	})
})(jQuery)
$(function(){
	// 导航页面跳转
	var url =['index.html','play_back.html','backup.html','device.html','log.html']
	$('div.top_nav li').each(function(index){
		$(this).on('click',function(){
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
	
	//表单全选..
	$('tbody.synCheckboxClick').SynchekboxClick();

	document.oncontextmenu = function(e){  //文档默认右键事件冒泡取消
		var e = e || window.event;
		if(e.target.tagName != 'BODY'){
			return;
		}
	}

	$('.hover').each(function(){  // 按钮元素添加鼠标事件对应样式
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
function showdata(id,type){  //显示表单下有ID的元素的val值
	return;
	var submit = $('#'+type).find('.confirm:visible').attr('id');
	var str =submit+'/'+id +'/';
	$('#'+type).find('input[id]').each(function(){ 
		str += $(this).attr('id')+':'+$(this).val()+'/';
	})
	debugData(str);
}
function debugData(data){  // 在ID为test的div元素中打印对象数据
	return;
	var index='default',
		str = 'Null';
	$('#test').html('');
	if(typeof(data) == 'number' || typeof(data) == 'string'){
		$('<span>'+index+'</span>:<span>"'+data+'"/</span>').prependTo($('#test'));
	}else{
		for(i in data){ 
			index = i;
			str = data[i];
			$('<span>'+index+'</span>:<span>'+str+'</span>/').prependTo($('#test'));
		}
	}
}

//弹出框部分操作
function closeMenu(){ 
	$('#iframe,div.confirm,div.menu').hide();
	$('#confirm').find('h4,span').html('');
	$('#menusList div.menu').find('input.data')
		.remove()
		.end().find(':text').val('');
		/*.end().end().each(function(){
			$(this).find('div.close:last').html(lang.Cancel);	
		})*/
	/*$('#menusList div.menu input.data').remove();
	$('#menusList div.menu input:text').val('');
	$('div.menu').each(function(){ 
		$(this).find('div.close:last').html(lang.Cancel);
	})*/
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
function returnTime(sInt){  //
	var H = parseInt(sInt)/3600;
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
	return fuckParseInt(a[0])*60*60+fuckParseInt(a[1])*60+fuckParseInt(a[2]);
}
// parseInt('08') == 0;  fuck it;
function fuckParseInt(str){
	var s = str.split('');
	if(parseInt(s[0]) == 0){
		return parseInt(s[1]);
	}else{ 
		return parseInt(str);
	}
} 
function checkHasObj(oSil,obj){
	var b = 0;			
	oSil.each(function(){
		if($(this).is(obj)){
			return b=1;
		}
	})
	return b;
}


function T(){//语言切换
	var str = lang ? lang[arguments[0]] : arguments[0].replace('_',' ');
	if(arguments.length ==1){
		document.write(str);
	}else{
		str = hintTrans(arguments);
	};
	return str;
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