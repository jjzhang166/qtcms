function imitate_select(obj){
		obj.click(function(){
			obj.next('ul.option').toggle();
		})
		obj.next('ul.option').click(function(){ 
				$(this).hide();
			}).mouseleave(function(){ 
				$(this).hide();
			})
		obj.next('ul.option').find('li').click(function(){
			$(this).parent('ul.option').hide();
			$(this).parent('ul.option').prev('div.select').find('span').html($(this).find('a').html());
		})
}
function addMouseStyle(obj,action){
	var width = obj.width();
	var left,top;
	obj.hover(function(){
		left = document.all ? parseInt(obj.css('backgroundPositionX')) : parseInt(obj.css('background-position').split('px')[0]);
		top = document.all ? parseInt(obj.css('backgroundPositionY')) : parseInt(obj.css('background-position').split('px')[1]);
		if(left == -30){
			//obj.css('background-position',left-width+'px'+' '+top+'px');
		}else{
			obj.css('background-position',left-width+'px'+' '+top+'px');
		}
	},function(){
		obj.css('background-position',left+'px'+' '+top+'px');
	}).mousedown(function(){
		$(window).off();
		if(left == -30){
			obj.css('background-position',left-width+'px'+' '+top+'px');
		}else{
			obj.css('background-position',left-(2*width)+'px'+' '+top+'px');
		}
	}).mouseup(function(){
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
			$('#operating li[class*="_setViewNum"]').each(function(){
				var T = document.all ? parseInt($(this).css('backgroundPositionY')) : parseInt($(this).css('background-position').split('px')[1]);
				$(this).css('background-position','0px'+' '+T+'px')	
			})
			var key = obj.attr('class').split('_')[0]
			autoImages(key,oView);	
			obj.css('background-position',-width+'px'+' '+top+'px');
			left=-30;
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
	$('div.dev_list').height(oLeft.height()-240);
	oBottom.width(oView.width()-4);
	$('#foot').css({
		top:oView.height()+236,
		width:oView.width()+238
	})
	$('#top').width(oView.width()+238);
	$('ul.table li').each(function(index){
			var W =  $(this).width() -14;
			var oDivs = $(this).find('div');
			if(type == 'preview'){
				oDivs.eq(0).width(W*0.19);
				oDivs.eq(1).width(W*0.71);
				oDivs.eq(2).width(W*0.1+8);
			}else{
				if(oDivs.length > 24){
					oDivs.width(W*0.037);
				}else{
					oDivs.eq(1).width(W-W*0.082-16);
				}
				oDivs.eq(0).width(W*0.082);
				
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
