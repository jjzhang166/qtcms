
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
				oSwitch.each(function(){
				var T = document.all ? parseInt($(this).css('backgroundPositionY')) : parseInt($(this).css('background-position').split('px')[1]);
				$(this).css('background-position','0px'+' '+T+'px').css('color','#B5B5B6');
				var oView = $('#playback_view');
				if(oView.length != 0){
					autoImages(oSwitch.index(ev.target)+1,oView);
				}
			})
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
		width:WinW-oLeft.width()+4,
		height:WinH-oBottom.height()-110
	});
	if(oView.width()<830){
		oView.width(830);
	}
	if(oView.height()<450){
		oView.height(450);
	}
	var l = type == 'preview' ? 90 : 148;
	oLeft.css({
		left:oView.width(),
		height:oView.height()+ l
	});
	$('div.dev_list').height(oLeft.height()-258);
	oBottom.width(oView.width()-4);
	$('#foot').css({
		top:oView.height()+236,
		width:oView.width()+238
	})
	$('#top').width(oView.width()+238);
	$('ul.table li').each(function(index){
			var oDivs = $(this).find('div');
			if(type == 'preview'){
				var W =  $(this).width() -14;
				oDivs.eq(0).width(W*0.19);
				oDivs.eq(1).width(W*0.71);
				oDivs.eq(2).width(W*0.1+8);
			}else{
				var W =  $(this).width() -50;
				if(oDivs.length > 24){
					oDivs.width((W-80)/24);
					oDivs.eq(0).width(80);
				}else{
					oDivs.eq(1).width(W - 60);
				}				
			}
	})
	autoImages(2,oView);
}

function autoImages(num,obj){
	if(nViewNum > num){
		$('#playback_view div').slice(num*num).remove();
	}else if(nViewNum < num){
		for(var i=0;i<(num*num)-(nViewNum*nViewNum);i++){
		$('<div><p>window 0'+(i+1)+'</p><img class="view_bg" src="images/view.jpg"><img class="no_view" src="images/No_Vedio.png"></div>').appendTo(oView);
		}	
	}	
	$('#playback_view img.view_bg').css({
		width:obj.width()/num - num*2,
		height:(obj.height()-num*14)/num -num*2
	})
	$('img.no_view').css({
		left:obj.width()/(2*num)-30,
		top:(obj.height()-num*16)/(num*2)+15
	})
	nViewNum = num;
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
						$(document).one('click',function(){ 
							option.hide();
						})
					}
				});
			})
			option.find('li').click(function(){
				This.next('ul.option').hide();
				This.find('span').html($(this).find('a').html());
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