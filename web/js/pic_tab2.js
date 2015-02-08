// JavaScript Document
$.fn.focusPic = function(defualt){
	defualt = $.extend({
		  box:null,//总框架
		  maxbox:null,//大图框架
		  minbox:null,//小图框架
		  minPrev:null,//小图左箭头
		  minNext:null,//小图右箭头
		  maxPrev:null,//大图左箭头
		  maxNext:null,//大图右箭头
		  autoplay:false,//是否自动播放
		  interTime:5000,//图片自动切换间隔
		  delayTime:800,//切换一张图片时间
		  order:0,//当前显示的图片（从0开始）
		  Maxpicdire:true,//大图滚动方向（true水平方向滚动）
		  Minpicdire:true,//小图滚动方向（true水平方向滚动）
		  minPicshowNum:null,//小图显示数量
    },defualt||{});

  var picNum = $(defualt.maxbox).find('ul li').length //图片的数量 
 /* var picMaxW = $(defualt.maxbox).find('ul li').outerWidth(true); //每一大图的宽度（包括padding margin border）
  var picMaxH = $(defualt.maxbox).find('ul li').outerHeight(true); //每一大图的高度（包括padding margin border）
  var picMinW = $(defualt.minbox).find('ul li').outerWidth(true); //每一小图的宽度（包括padding margin border）
  var picMinH = $(defualt.minbox).find('ul li').outerHeight(true); //每一小图的高度（包括padding margin border）*/
  var picMaxW = $(defualt.maxbox).find('ul li').width(); //每一大图的宽度（包括padding margin border）
  var picMaxH = $(defualt.maxbox).find('ul li').height(); //每一大图的高度（包括padding margin border）
  var picMinW = $(defualt.minbox).find('ul li').width(); //每一小图的宽度（包括padding margin border）
  var picMinH = $(defualt.minbox).find('ul li').height(); //每一小图的高度（包括padding margin border）*
  var pictime;  //自动播放的setInterval()的对象
  var maxPicindex=0;
  var minPicindex=0;
  $(defualt.maxbox).find('ul').width(picNum*picMaxW).height(picMaxH);
  $(defualt.minbox).find('ul').width(picNum*picMinW).height(picMinH);
  
	 //自动播放
	  if(defualt.autoplay==true){
		pictime = setInterval(function(){
			maxShow(maxPicindex);
			minShow(minPicindex)
			maxPicindex++;
			minPicindex++;
			if(maxPicindex==picNum){maxPicindex=0};	
			if(minPicindex==picNum){minPicindex=0};
				  
	  },defualt.interTime);	
	  
	  //鼠标经过停止播放
	  $(defualt.box).hover(function(){
		  clearInterval(pictime);
	  },function(){
		pictime = setInterval(function(){
			maxShow(maxPicindex);
			minShow(minPicindex);
			maxPicindex++;
			minPicindex++;
			if(maxPicindex==picNum){maxPicindex=0};	
			if(minPicindex==picNum){minPicindex=0};
				  
			  },defualt.interTime);			
		  });
	  }

    //点击小图切换大图
    $(defualt.minbox).find('li').click(function () {
        maxPicindex = minPicindex = $(defualt.minbox).find('li').index(this);
        maxShow(maxPicindex);
		minShow(minPicindex);
    }).eq(defualt.order).trigger("click");
  
 
    //大图左右切换	
	$(defualt.maxPrev).click(function(){
		if(maxPicindex==0){maxPicindex=picNum};
		if(minPicindex==0){minPicindex=picNum};
		minPicindex--;
		maxPicindex--;
		maxShow(maxPicindex);
		minShow(minPicindex);	
		})
	$(defualt.maxNext).click(function(){
		if(maxPicindex==picNum-1){maxPicindex=-1};
		if(minPicindex==picNum-1){minPicindex=-1};
		minPicindex++;
		minShow(minPicindex)
		maxPicindex++;
		maxShow(maxPicindex);
		})
    
	//小图左右切换	
	$(defualt.minPrev).click(function(){
		if(minPicindex==0){minPicindex=picNum};
		if(maxPicindex==0){maxPicindex=picNum};
		minPicindex--;
		maxPicindex--;
		maxShow(maxPicindex);
		minShow(minPicindex);	
		})
	$(defualt.minNext).click(function(){
		if(maxPicindex==picNum-1){maxPicindex=-1};
		if(minPicindex==picNum-1){minPicindex=-1};
		minPicindex++;
		minShow(minPicindex)
		maxPicindex++;
		maxShow(maxPicindex);
		})
		
		
	
    //大图切换过程
	function maxShow(maxPicindex){
		var gdjl_w=-maxPicindex*picMaxW;
		var gdjl_h=-maxPicindex*picMaxH;
		if(defualt.Maxpicdire==true){
			$(defualt.maxbox).find('ul li').css('float','left');
			$(defualt.maxbox).find('ul').stop().animate({'left':gdjl_w},defualt.delayTime);
		}else{
		  $(defualt.maxbox).find('ul').stop().animate({'top':gdjl_h},defualt.delayTime);
		}//滚动
		//$(defualt.maxbox).find('ul li').eq(maxPicindex).fadeIn(defualt.delayTime).siblings('li').fadeOut(defualt.delayTime);//淡入淡出
		$(defualt.minbox).find('li').eq(minPicindex).addClass("on").siblings(this).removeClass("on");
		var data = $(defualt.minbox).find('li').eq(minPicindex).data('data');
		
		for(var i in data){
			var str1 = 'pic'+i;
			if(i=='type'){
				var str;
			   switch(data[i]){
				case '0':
				   str = _T('Preview');
				   break;
				case '1':
				   str = _T('Local_Playback');
				   break;
				case '2':
				   str = _T('Remote_Playback');
				   break;
			  }
			  $('#'+str1).html(str);
			}else{
				$('#'+str1).html(data[i]);
			}
			
			//alert(data[i]);
		}
	};
  //小图切换过程
	function minShow(minPicindex){
		var mingdjl_num =minPicindex-defualt.minPicshowNum+2
		var mingdjl_w=-mingdjl_num*picMinW;
		var mingdjl_h=-mingdjl_num*picMinH;
		
		if(defualt.Minpicdire==true){
			$(defualt.minbox).find('ul li').css('float','left');
			if(picNum>defualt.minPicshowNum){
				if(minPicindex<3){mingdjl_w=0;}
				if(minPicindex==picNum-1){mingdjl_w=-(mingdjl_num-1)*picMinW;}
				$(defualt.minbox).find('ul').stop().animate({'left':mingdjl_w},defualt.delayTime);
				}
		}else{
			$(defualt.picNum).find('ul li').css('float','none');
			if(picNum>defualt.minPicshowNum){
				if(minPicindex<3){mingdjl_h=0;}
				if(minPicindex==picNum-1){mingdjl_h=-(mingdjl_num-1)*picMinH;}
				$(defualt.minbox).find('ul').stop().animate({'top':mingdjl_h},defualt.delayTime);
				}
			}
		
	}
}