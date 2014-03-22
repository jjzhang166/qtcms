var oLeft,oBottom,oView,oPlayBack,oPlaybacKLocl,oDiv;
var	nViewNum = 0,
	NowMonth = 0,
	drag_timer = null,
	oSelected = [];
	$(function(){
		oLeft = $('#search_device');
		oBottom = $('#operating');
		oView = $('#playback_view');
		oPlayBack = $('#playback')[0];
		oPlaybackLocl = $('#playbackLocl')[0];
		oDiv = $('div.dev_list');
		ViewMax();


	    
	    $('ul.filetree').treeview();		
		
		oDiv.eq(1).hide();
		
		$('.hover').each(function(){  // 按钮元素添加鼠标事件对应样式
			var action = $(this).attr('class').split(' ')[0];
			addMouseStyle($(this),action);
		})

		var listParent = $('div.dev_list');
		listParent.on('click','li:has(span.device):gt(0)',function(){  //设备单击初始化部分样式
			var obj = $(this)
			$('div.dev_list li').not(obj).removeClass('sel');
			obj.addClass('sel');
			PBrecFileTableInit();
		})

		listParent.on('dblclick','li:has(span.device):gt(0)',function(){ //设备双击开始搜索
			searchVideo();
		})

		/*$('div.dev_list span.channel').on('click',function(){
			var b = true;
			var obj = $(this).parent('li')
			var oSibs = obj.siblings().add(obj);
			$('div.dev_list li').filter(function(){
				if(!checkHasObj(oSibs,$(this))){
					return $(this);
				} 			
			}).removeClass('sel');

			obj.toggleClass('sel');

			obj.siblings().add(obj).each(function(){
				if(!$(this).hasClass('sel')){
					b = false;
				}
			})

			if(b){
				obj.parent('ul').parent('li').addClass('sel').find('span.device').addClass('sel');
			}else{ 
				obj.parent('ul').parent('li').removeClass('sel').find('span.device').removeClass('sel');
			}
		})*/
		
		var channelvideo = $("#channelvideo")
		channelvideo.on('click','input:checkbox',function(event){   //录像文件列表选择通道不能超过4个
			event.stopPropagation();
			if($(this).prop('checked')){
				oSelected.push($(this));
			}else{ 
				oSelected.pop($(this));
			}
			if(oSelected.length>4){
				oSelected.shift().prop('checked',false);
			}
		})

		channelvideo.on('click','td.no_border',function(event){ 
			$(this).find('input:checkbox').click();
		})

		channelvideo.on({  //整个搜索的文件列表事件
			mousedown:function(event){
				try{
					dragStopMove();
					oPlaybackLocl.GroupStop();
					oPlayBack.GroupStop();
					$('#togglePlay').removeAttr('toggle').removeAttr('hasFile').css('background-position','0px 0px');
				}catch(e){
					//alert('try:'+e);
				};
				var left = event.pageX
			    	if(left < 79){
			    		return false;
			    	}
				//event.stopPropagation();
				var moveObj = $('div.play_time').css('left',left-2);
				set_drag(81,$(this).width(),moveObj);
			}
		})

		$('div.play_time').on({  //文件搜索的下的事件滑动条事件
			dblclick:function(event){
				playVideo();
			},
			mousedown:function(){
				set_drag(81,channelvideo.width(),$('div.play_time'));
			}	
		});
		
		$(window).resize(function(){  //窗口自适应大小
			ViewMax();	
		})

		$('#type').next('ul').find('a').each(function(index){  //搜索文件类型下拉菜单
			$(this).click(function(){
				$('#type span').attr('type',index);
			})
		})
		$('#nowSearchType input:radio').each(function(index){  //全局变量控制远程或本地搜索
			$(this).click(function(){
				bool = index;
			})
		})
		oPlayBack.AddEventProc('RecFileInfo','RecFileInfoCallback(data)');
		oPlayBack.AddEventProc('recFileSearchFinished','RecfinishCallback(data)');
		oPlaybackLocl.AddEventProc('GetRecordFile','RecFileInfoCallback(data)');
		oPlaybackLocl.AddEventProc('GetRecordFile','RecfinishCallback(data)');

	})///

	function togglePlay(){ 
		var obj = $('#togglePlay');
		var to = $('#togglePlay').attr('toggle'),
			hasFile = $('#togglePlay').attr('hasFile');
			speed = $('#togglePlay').attr('speed');
		if(hasFile){
			if(to){
				/*if(speed){
					alert('正常速度');
					GroupSpeedNormal();
				}else{*/
					//alert('继续');
					playAction('GroupContinue');
				//}
			}else{
				//alert('暂停');
				playAction('GroupPause')
			}
		}else{
			//alert('播放');
			playVideo();		
		}
	}
	function playVideo(){
		dragStopMove();
		try{
			nowSpeed = 1;
			var obj = $('#togglePlay');
				obj.attr({
					toggle:'1',
					hasFile:'1'
				}).css('background-position','0px'+' '+(-obj.height())+'px');
			oPlaybackLocl.GroupStop();
			oPlayBack.GroupStop();
		}catch(e){
			//alert('try:'+e);
		};
		var begin = getDragSart($('#channelvideo').width()-2,$('div.play_time').offset().left,$("div.calendar span.nowDate").html()),
			date = $("div.calendar span.nowDate").html(),
			end = date+' 23:59:59';
			setDevData2ocx();
		if(bool){
			$("#channelvideo").find('input:checkbox').each(function(index){
				if($(this).is(':checked')){
					var filepath = $('div.dev_list li.sel').find('span.channel').eq(index).data('filepath');
					if(filepath){
						if(oPlaybackLocl.AddFileIntoPlayGroup(filepath,index,begin,end) != 0){
							b = false;
						};
					}
				}
			});
			oPlaybackLocl.GroupPlay();
		}else{
			var type = parseInt($('#type span').attr('type')),
			type = type == 0 ? 15 : 1 << type;
			oPlayBack.GroupPlay(type,begin,end);
		}
		dragStartMove();
	}
	function getDragSart(X2,left,date){
		return  date+' '+returnTime((left-81)/(X2-81)*24*3600);
	}
	function playAction(str){
		var obj = bool ? oPlaybackLocl : oPlayBack; //回放插件对象
			//alert(str+'::当前速度:'+(nowSpeed>1?nowSpeed:1/nowSpeed));
			if(bool && (str == 'GroupSpeedFast' || str == 'GroupSpeedSlow')){
				obj[str](nowSpeed>1?nowSpeed:1/nowSpeed);
			}else{
				obj[str]();
			}
	}
	var nowSpeed = 1;
	function playSpeed(str){
		var show='';
		var max = bool ? 8 : 2 ;
		if(str){
			nowSpeed = nowSpeed*2;
			nowSpeed = nowSpeed > max ? max : nowSpeed;
		}else{
			nowSpeed = nowSpeed/2;
			nowSpeed = nowSpeed < (1/max) ? (1/max) : nowSpeed;
		}
		if(nowSpeed == 1){
			//alert(nowSpeed+'==');
			playAction('GroupSpeedNormal');
			show='1X';
		}else if(nowSpeed<1){
			playAction('GroupSpeedSlow');
			show='1/'+(1/nowSpeed)+'X';
		}else{
			playAction('GroupSpeedFast');
			show=nowSpeed+'X';
		}
		palybackspeed(show);
	}
	function groupStop(){
		$('#togglePlay').removeAttr('hasFile').css('background-position','0px 0px');
		dragStopMove();
		var obj = bool ? oPlaybackLocl : oPlayBack;
		obj.GroupStop();
	}

	function palybackspeed(str){
		$('#palybackspeed').html(str);
	}


	var color = [];
		color[1] = '#7BC345';
		color[2] = '#FFE62E';
		color[4] = '#F00';
		color[8] = '#F78445';
	function VideoData2Ui(obj){  // CGI 数据填充.
		obj.each(function(){ 
			var chlData = $(this).html().split('|'); //disk(int)|session(int)|chn(int)|type(int)|begin(time_t)|end(time_t)
			var startDate = $('div.calendar span.nowDate').html().split('-');
			var start = parseInt(chlData[4])-Date.UTC(startDate[0],parseInt(startDate[1])-1,startDate[2])/1000;
			var p = ($('#channelvideo').width()-80)/(3600*24);
			var width = (chlData[5]-chlData[4])*p
			var left = start*p+81;
			$('<div class="video" style="background:'+color[chlData[3]]+';left:'+left+'px; width:'+width+'px;"></div>').appendTo('#channelvideo tr:eq('+(parseInt(chlData[2]))+')');
		})		
	}

	function RecFileInfoCallback(data){
		var start = data.start || data.startTime;
			start=time2Sec(start.split(' ')[1]);
		var end = data.end || data.stopTime;
			end = time2Sec(end.split(' ')[1]);
		var chl = data.channel || parseInt(data.channelnum -1);
		var p = ($('#channelvideo').width()-80)/(3600*24);
		var width = (end-start)*p;
		var left = start*p+81;
		var types = data.types || 8;
		if(data.filepath){
			var oChannel = $('div.dev_list li.sel').find('span.channel').eq(chl);
			var filepathArr = oChannel.data('filepath');
				filepathArr = filepathArr ? filepathArr.toString().split(',') : [];
				filepathArr.push(data.filepath);
				filepathArr.sort(SortByfileTime).join(',');
			oChannel.data('filepath',filepathArr);			
		}
		//alert(data.type+'+'+data.channel);
		$('<div class="video" style="background:'+color[types]+';left:'+left+'px; width:'+width+'px;"></div>').appendTo('#channelvideo tr:eq('+chl+')');
		showRecProgress(parseInt(data.index)+1);
		/*$('<div class="video" style="background:#F78445;left:100px; width:60px;"></div>').appendTo('#channelvideo tr:eq('+(parseInt(data.channel))+')');*/

	}
	function SortByfileTime(a,b){  //文件路径时间升序排列
		var reg = /.*?(\d{6})\.avi/g;
		var a = parseInt(a.replace(reg,'$1'));
		var b = parseInt(b.replace(reg,'$1'));
		return a - b;
	}
	function dragStartMove(){
		var SynTimeUnits = 1000;//nowSpeed<1 ? 1000*nowSpeed:1000/nowSpeed;
		var oPlay = bool ? oPlaybackLocl : oPlayBack;
		//return false;
		var oDrag=$('div.play_time');
		var initleft = parseInt(oDrag.offset().left);
		var max = $('#channelvideo').width();
		var p = (max-79)/(3600*24);
		drag_timer = setInterval(function(){
			var nowPlayd = parseInt(oPlay.GetNowPlayedTime());
			var left = initleft+p*nowPlayd;
			//show(bool+'//oxcoPlay:'+$(oPlay).attr('id')+'//初始左边距:'+initleft+'像素//当前以播放时间:'+nowPlayd+'秒//当前走过:'+p*nowPlayd+'像素//当前刷新速度:'+SynTimeUnits+'毫秒//速度'+nowSpeed);
			if(left >= max-2){ 
				left=max-2;
				dragStopMove();
			}
			oDrag.css('left',left);
			//showNowPlayBackTime($('#now_time'),left,max);
		},SynTimeUnits);
	}
	function dragStopMove(){
		clearInterval(drag_timer);
	}
	//回放页面文件显示表格初始化
	function PBrecFileTableInit(){
		oSelected = [];
		var odev = $('div.dev_list li.sel span.channel')
		var oVideoList = $("#channelvideo").html('');
		if(odev.length != 0){
			odev.each(function(index){
				var name = $(this).data('data').channel_name;
				var str = index < 4 ? 'checked="checked"' : '';
				addRecFileTable(str,name);
			})
		}
		if(odev.length < 4){
			for(var i=2;i<=4;i++)
			addRecFileTable('disabled="disabled"','chl_'+i);
		} 

		setTables();

		$("#channelvideo input:checkbox:checked").each(function(){
			oSelected.push($(this));
		});
	}

	function addRecFileTable(str,name){
		$('<tr><td class="no_border"><input type="checkbox"'+str+'>'+name+'</td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td></td><td></tr>').appendTo($("#channelvideo"))
	}
