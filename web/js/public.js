
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
		height:oView.height()+120
	});
	$('div.dev_list').height(oLeft.height()-294);
	oBottom.width(oView.width());
	$('#foot').css({
		top:oView.height()+212
	})
	$('table.table tr').each(function(index){
			var oTds = $(this).find('td');
			if(type == 'preview'){
				var W =  $('table.table').width() -14;
				oTds.eq(0).width(W*0.19);
				oTds.eq(1).width(W*0.71);
				oTds.eq(2).width(W*0.1+8);
			}else{
				var W =  $('table.table').width() -50;
				oTds.width((W-80)/24);
				oTds.eq(0).width(80);			
			}
	})
}

function set_drag(oDrag,X1,X2){
	var disX = 0;
	oDrag.mousedown(function(event){
		var event = event || window.event;
		disX = event.clientX - oDrag.offset().left;

		$(document).mousemove(function(event){
			var event = event || window.event;
				var left = event.clientX - disX;
			    left = left < X1 ? X1 : left;
				left = left > X2 ? X2 : left;
			oDrag.css('left',left+'px');
			if(oDrag.prop('nodeName') == 'SPAN'){
				$('p.now').width(left-X1);
			}
		});
		$(document).mouseup(function(){
			$(document).off();
		})
		return false;
	})
}
(function($){
	$.fn.extend({
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
					This.find('#vendor_ID').val($(this).html());
					This.find('input:hidden').val($(this).attr('key'));
				}
			})
		}
	})
})(jQuery)
$(function(){
	$('ul.ope_list').each(function(){
		$(this).toSwitch();
	});
	$('div.select').each(function(){
		$(this).toSelect()
	});

	$('#top div.top_nav li').mousedown(function(){ 
		$(this).css('background-position','0 -52px');
	}).mouseup(function(){ 
			$(this).css('background-position','0 0');
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
function showdata(id,type){ 
	var submit = $('#'+type).find('.confirm:visible').attr('id');
	var str =submit+'/'+id +'/';
	$('#'+type).find('input[id]').each(function(){ 
		str += $(this).attr('id')+':'+$(this).val()+'/';
	})
	show(str);
}