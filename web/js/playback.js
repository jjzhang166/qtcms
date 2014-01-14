var oLeft,oBottom,oView,oPlayBack,oPlaybacKLocl,
	nViewNum = 0,
	NowMonth = 0,
	num = 0;
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
		$('div.dev_list span.device').click(function(){
			$('div.dev_list span.device').removeClass('sel');
			$(this).addClass('sel');
			
			oSelected = [];

			var oVideoList = $("#channelvideo")
			oVideoList.find('tr:gt(3)').remove()
					  .end().find('input:checkbox').prop('disabled',false);
			var count = oDevData['channel_count']
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
				var left = event.pageX
			    	if(left < 79){
			    		return false;
			    	}
				//event.stopPropagation();
				$('div.play_time').css('left',left-2);
				set_drag(0,79,$('#channelvideo').width());
			}/*,
			mouseup:function(event){ 
				playVideo(event.pageX);
			}*/
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
		$('div.dev_list').on('click','span.channel',function(){ 
			alert($(this).data('filepath'));
		})
	})///

	function searchVideo(){
		var seletDev = $('div.dev_list span.device.sel');
		if(seletDev.length == 0){
			$('div.dev_list span.device:first').addClass('sel');
		}
		$('#channelvideo div.video').remove();
		  //cgi 请求数据
		/*var channels = 0;   
		$('#channelvideo input:checkbox').each(function(index){ 
			if($(this).is(':checked')){
				channels += 1 << index;
			}
		});
		var type = parseInt($('#type span').attr('type'))
			type = type == 0 ? 15 : 1 << type;
		var date = $("div.calendar span.nowDate").html();
		var begin = gettime($('div.timeInput:eq(0) input'));
		var end = gettime( $('div.timeInput:eq(1) input'));
		var url ='http://'+devData['address']+':'+devData['http'];
		var num=0;
		var page = 100;
		getVideoData(num);

		function getVideoData(num){ 
			var xmlstr = '<juan ver="0" squ="fastweb" dir="0"><recsearch usr="' + devData['username'] + '" pwd="' + devData['password'] + '" channels="' + channels + '" 	types="' + type + '" date="' + date + '" begin="' + begin + '" end="' + end + '" session_index="'+num+'" session_count="'+page+'" /></juan>';
			$.ajax({ 
			type:"GET",
			url: url + "/cgi-bin/gw.cgi?f=j",
			data: "xml=" + xmlstr, 
			dataType: 'jsonp',
			jsonp: 'jsoncallback',
			success: function(data, textStatus){
				VideoData2Ui($('s',data.xml))
				if($('recsearch',data.xml).attr('session_total')>(num+page)){
					num += page;
					getVideoData(num);
				}
			}
		});	
		}*/
		ocxsearchVideo();
		
	}
	function playVideo(){ 
		setDevData2ocx();
		var bool=$('#search_device div.switchlist:eq(1) li.switchlistAct').index();
		var begin = getDragSart(),
			date = $("div.calendar span.nowDate").html(),
			end = date+' 23:59:59';
		if(bool){
			oPlayBack.GroupStop();
			var type = parseInt($('#type span').attr('type')),
			type = type == 0 ? 15 : 1 << type;
			oPlayBack.GroupPlay(type,begin,end);
		}else{
			oPlaybackLocl.GroupStop();
			$("#channelvideo").find('input:checkbox').each(function(index){
				if($(this).is(':checked')){
					var filepath = $('div.dev_list span.device.sel').parent('li').find('span.channel').eq(index).data('filepath');
					if(filepath){
						if(oPlaybackLocl.AddFileIntoPlayGroup(filepath,index,begin,end) != 0){
							alert('设备'+oDevData.name+'下的通道'+index+'的本地回放数据写入到窗口失败');
							b = false;
						};
					}
				}
			});
			oPlaybackLocl.GroupPlay();
		}	
	}
	function getDragSart(){
		var	X1 = 79,
			X2 = $('table.table').width() -42,
			left = $('div.play_time').offset().left,
			date = $("div.calendar span.nowDate").html(),
			sScond = parseInt(((left-X1)/(X2-X1)*24*3600)),
			begin = date+' '+returnTime(sScond);
			return begin;
	}
	function playAction(str){ 
		var bool=$('#search_device div.switchlist:eq(1) li.switchlistAct').index();
		var obj = {};  //回放插件对象
		if(bool){ 
			obj = oPlayBack;
		}else{ 
			obj = oPlaybackLocl;
		}
		obj[str]();
	}
	function palybackspeed(str){ 
		$('#palybackspeed').html(str);
	}
	function setDevData2ocx(){
		var oDevData = $('div.dev_list span.device.sel').data('data');
		var b = true;
		var bool=$('#search_device div.switchlist:eq(1) li.switchlistAct').index();
		if(bool){
			if(oPlayBack.setDeviceHostInfo(oDevData.address,oDevData.port,oDevData.eseeid)){ 
				alert('IP地址设置失败或者端口不合法!');
				b = false;
			}
			if(oPlayBack.setDeviceVendor(oDevData.vendor)){
				alert('vendor为空设置失败!');
				b = false;
			}
			oPlayBack.setUserVerifyInfo(oDevData.username,oDevData.password);
			$("#channelvideo").find('input:checkbox').each(function(index){
				if($(this).is(':checked')){
					var state = oPlayBack.AddChannelIntoPlayGroup(index,index);
					alert('当前窗口:'+index+'绑定通道 '+index+'的数据 ,完成状态为: '+state);
					if(state){
						b = false;
					};
				}
			});
		}else{ 
			if(oPlaybackLocl.SetSynGroupNum(4)){ 
				alert('同步组数量设置失败');
				b = false
			}
		}
		return b;
	}

	var typeHint = [];
		typeHint[1] = '定时';
		typeHint[2] = '运动';
		typeHint[4] = '警告';
		typeHint[8] = '手动';
		typeHint[15] = '全部';
	function ocxsearchVideo(){
		setDevData2ocx();
		var devData = $('div.dev_list span.device.sel').data('data');
		var bool=$('#search_device div.switchlist:eq(1) li.switchlistAct').index()
		var type = $('#type span').attr('type');
			type = type == 0 ? 15 : 1 << type;
		var date = $("div.calendar span.nowDate").html();
		var startTime =date+' '+gettime($('div.timeInput:eq(0) input'));
		var endTime =date+' '+gettime($('div.timeInput:eq(1) input'));
		/*show(chl+'+'+type+'+'+startTime+'+'+endTime);
		alert(oPlayBack.startSearchRecFile(chl,type,startTime,endTime));*/
		if(bool){
			/*oPlaybackLocl.style.height='0px';
			oPlayBack.style.height='100%';*/
			oPlaybackLocl.GroupStop();
			var chl = 0;
			for (var i=0;i<devData.channel_count;i++){
				chl += 1 << i;
			};
			if(oPlayBack.startSearchRecFile(chl,type,startTime,endTime)!=0){
				alert('控件检索设备'+devData.name+'的'+typeHint[type]+'录像失败');
			}
		}else{
			/*oPlayBack.style.height='0px';
			oPlaybackLocl.style.height='100%';*/
			oPlayBack.GroupStop();
			var chl ='';
			for (var i=1;i<=devData.channel_count;i++){
				chl+=i+';';
			};
			if(oPlaybackLocl.searchDateByDeviceName(devData.name)){ 
				alert('设备'+devData.name+'在本地没有录像!  '+oPlaybacKLocl.searchDateByDeviceName(devData.name));
				return false;
			}
			oPlaybackLocl.searchVideoFile(devData.name,date,gettime($('div.timeInput:eq(0) input')),gettime($('div.timeInput:eq(1) input')),chl);
		}
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
