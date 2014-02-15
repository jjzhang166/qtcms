var oLeft,oBottom,oView,oPlayBack,oPlaybacKLocl,
	nViewNum = 0,
	NowMonth = 0
	drag_timer = null;
	$(function(){
		oLeft = $('#search_device');
		oBottom = $('#operating');
		oView = $('#playback_view')
		oPlayBack = $('#playback')[0];
		oPlaybackLocl = $('#playbackLocl')[0];
		ViewMax();
		var oAs = $('ul.dev_list_btn li'),
			oDiv = $('div.dev_list');
	    
	    $('ul.filetree').treeview();		
		
		oDiv.eq(1).hide();
		
		$('.hover').each(function(){  // 按钮元素添加鼠标事件对应样式
			var action = $(this).attr('class').split(' ')[0];
			addMouseStyle($(this),action);
		})

		oAs.each(function(index){
			$(this).on('click',function(){
				$(window).off();
				oDiv.hide();
				oDiv.eq(index).show();
			})
		})

		$('div.menu .close').click(function(){  //弹出操作框下部分元素添加关闭窗口事件
			closeMenu();
		})

		var oSelected = [];
		/*$('div.dev_list').on('click','span.device',function(){
			$('div.dev_list span.device').removeClass('sel');
			$(this).toggleClass('sel');
			if($(this).parent('li').hasClass('sel')){ 
				$(this).parent('li').find('li').addClass('sel');
			}else{
				$(this).parent('li').find('li').removeClass('sel');
			}
		})*/
		$('div.dev_list span.device').on({
			click:function(){
				$('div.dev_list span.device').removeClass('sel');
				$(this).addClass('sel');
				
				oSelected = [];

				var oVideoList = $("#channelvideo")
				oVideoList.find('tr:gt(3)').remove()
						  .end().find('input:checkbox').prop('disabled',false);
				var count = $(this).data('data').channel_count;
				if(count<4){
					oVideoList.find('input:checkbox:gt('+(count-1)+')').prop({disabled:true,checked:false});
				}else{ 
					oVideoList.find('input:checkbox:lt(4)').prop({checked:true});
				}
				for(var i=5; i<=count;i++){
					var num = addZero(i);
					$('<tr><td class="no_border"><input type="checkbox">window '+num+'</td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td></td><td></tr>').appendTo($("#channelvideo"))
				}

				setTables();

				$("#channelvideo input:checkbox:checked").each(function(){
					oSelected.push($(this));
				});
			},
			dblclick:function(){
				searchVideo();
			}
		})

		$("#channelvideo").on('click','input:checkbox',function(event){ 
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

		$("#channelvideo").on('click','td.no_border',function(event){ 
			$(this).find('input:checkbox').click();
		})

		$("#channelvideo").on({ 
			mousedown:function(event){
				dragStopMove();
				var left = event.pageX
			    	if(left < 79){
			    		return false;
			    	}
				//event.stopPropagation();
				$('div.play_time').css('left',left-2);
				set_drag(0,79,$('#channelvideo').width());
			},
			dblclick:function(event){ 
				playVideo(event.pageX);
			}
		})
		
		$(window).resize(function(){
			ViewMax();	
		})

		oView.on({
			mouseover:function(){
				$(this).addClass('view_hover');
			},
			mouseleave:function(){
				$(this).removeClass('view_hover');
			}
		},'div');

		$('div.calendar').initCalendar();

		$('#type').next('ul').find('a').each(function(index){ 
			$(this).click(function(){
				$('#type span').attr('type',index);
			})
		})
		
		oPlayBack.AddEventProc('RecFileInfo','RecFileInfoCallback(data)');
		oPlaybackLocl.AddEventProc('GetRecordFile','RecFileInfoCallback(data)');
	})///

	function playVideo(){ 
		dragStopMove();
		try{
			oPlaybackLocl.GroupStop();
			oPlayBack.GroupStop();
		}catch(e){
			//alert('try:'+e);
		};
		var bool=$('#nowSearchType li.switchlistAct').attr('now');
		var begin = getDragSart(),
			date = $("div.calendar span.nowDate").html(),
			end = date+' 23:59:59';
			//show(begin+'//'+end);
			setDevData2ocx(bool);
		if(bool == 1){
			var type = parseInt($('#type span').attr('type')),
			type = type == 0 ? 15 : 1 << type;
			oPlayBack.GroupPlay(type,begin,end);
		}else{
			$("#channelvideo").find('input:checkbox').each(function(index){
				if($(this).is(':checked')){
					var filepath = $('div.dev_list span.device.sel').parent('li').find('span.channel').eq(index).data('filepath');
					if(filepath){
						if(oPlaybackLocl.AddFileIntoPlayGroup(filepath,index,begin,end) != 0){
							b = false;
						};
					}
				}
			});
			oPlaybackLocl.GroupPlay();
		}
		dragStartMove();
	}
	function getDragSart(){
		var	X1 = 79,
			X2 = $('#channelvideo').width()-$('#channelvideo .no_border').width(),
			left = $('div.play_time').offset().left,
			date = $("div.calendar span.nowDate").html(),
			sScond = parseInt(((left-X1)/(X2-1)*24*3600)),
			begin = date+' '+returnTime(sScond);
			return begin;
	}
	var up = 1,down=1;
	function playAction(str){ 
		var bool=$('#search_device div.switchlist:eq(1) li.switchlistAct').index();
		var obj = {};  //回放插件对象
		if(bool){ 
			obj = oPlayBack;
		}else{ 
			obj = oPlaybackLocl;
		}
		if(str == 'GroupSpeedFast' || str == 'GroupSpeedSlow'){
			var show ='';
			if(obj.id == 'playback'){
				if(str == 'GroupSpeedFast'){
					down = 1;
					up = 2;
					show = '2x';
				}else{ 
					down = 2;
					up = 1;
					show = '1/2x';
				}		
				obj[str]();
			}else if(obj.id == 'playbackLocl'){
				var speed = 1;
				if(str == 'GroupSpeedFast'){
					down = 1;			
					up *= 2;
					up = up > 8 ? 8:up;
					speed = up;
				}else{
					up = 1;
					down *= 2;
					down = down > 8 ? 8:down;
					show = '1/';
					speed = down;
				}		
				show = show+speed+'x';			
				obj[str](speed);
			}
			palybackspeed(show);
		}else{ 
			if(str == 'GroupSpeedNormal'){
				up = 1;
				down=1;
				palybackspeed('1x');
			}
			obj[str]();
		}
		dragStartMove();
	}
	function palybackspeed(str){
		dragStartMove();
		$('#palybackspeed').html('').html(str);
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
			var oChannel = $('div.dev_list span.device.sel').parent('li').find('span.channel').eq(chl);
			var filepathArr = oChannel.data('filepath');
				filepathArr = filepathArr ? filepathArr.toString().split(',') : [];
				filepathArr.push(data.filepath);
				filepathArr.sort(SortByfileTime).join(',');
			oChannel.data('filepath',filepathArr);			
		}
		//alert(data.type+'+'+data.channel);
		$('<div class="video" style="background:'+color[types]+';left:'+left+'px; width:'+width+'px;"></div>').appendTo('#channelvideo tr:eq('+chl+')');
		/*$('<div class="video" style="background:#F78445;left:100px; width:60px;"></div>').appendTo('#channelvideo tr:eq('+(parseInt(data.channel))+')');*/

	}
	function SortByfileTime(a,b){  //文件路径时间升序排列
		var reg = /.*?(\d{6})\.avi/g;
		var a = parseInt(a.replace(reg,'$1'));
		var b = parseInt(b.replace(reg,'$1'));
		return a - b;
	}
	function dragStartMove(){
		var SynTimeUnits = up == 1 ? 1000*down:1000/up;
		var oPlay = $('#nowSearchType li.switchlistAct').attr('now') ? oPlayBack : oPlaybackLocl;
		var oDrag=$('div.play_time');
		var initleft = parseInt(oDrag.offset().left);
		var p = ($('#channelvideo').width()-80)/(3600*24);
		var max = $('#channelvideo').width()-2;
		drag_timer = setInterval(function(){
			var nowPlayd = parseInt(oPlay.GetNowPlayedTime()) == 0 ? 1 : parseInt(oPlay.GetNowPlayedTime())
			var left = initleft+p*nowPlayd;
			try{show('初始左边距:'+initleft+'像素//当前走过:'+p*nowPlayd+'像素//当前刷新速度:'+SynTimeUnits+'毫秒');}catch(e){
				alert(e);
			}
			if(left >= max){ 
				left=max;
				dragStopMove();
			}
			oDrag.css('left',left);
		},SynTimeUnits);
	}
	function dragStopMove(){
		clearInterval(drag_timer);
	}
