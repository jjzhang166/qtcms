var oLeft,oBottom,oView,oPreView,oDiv;
var	nViewNum = 0;
var timer = null;
var winState=['已经接入了连接!','正在连接!','断开连接!','正在断开连接!'];
var currentWinStateChange = ['已连接!','正在连接!','已关闭!','正在关闭!'];
	$(function(){
		
		oLeft = $('#search_device');
		oBottom = $('#operating');
		oView = $('#playback_view');
		oPreView= $('#previewWindows')[0];
		$('#sound').prev('li').prop('soundOn',true);
		oDiv = $('div.dev_list');
		setViewMod(oCommonLibrary.getSplitScreenMode());

		var oAs = $('ul.dev_list_btn a');
	    
		$(window).off();

	    $('ul.filetree').treeview().not(':eq(0)').parent('div.dev_list').hide();
		
		$('.hover').each(function(){
			var action = $(this).attr('class').split(' ')[0];
			addMouseStyle($(this),action);
		})
		
		oAs.each(function(index){
			$(this).click(function(){
				$(window).off();
				oAs.removeClass('active');
				oDiv.hide();
				$(this).addClass('active');
				oDiv.eq(index).show();
			})
		})
		//控件最大化
		ViewMax('preview');
		
		$(window).resize(function(){
			ViewMax('preview');
		})
 		//打开通道
		oDiv.on('dblclick','span.channel',function(){ 
			//show($(this).data('data'));
			var chlData = getChlFullInfo($(this));
			if($(this).attr('state')){
				CloseWind($(this).attr('wind'),chlData.dev_id);
			}else{
				openWind(oPreView.GetCurrentWnd(),chlData);
			}
		})

		//打开设备下的说所有通道
		oDiv.on('dblclick','span.device',function(){ 
			var oDevice = $(this)
			var chlData;
			var wind = oPreView.GetCurrentWnd();
			if(oDevice.attr('bAllopen')){
				oDevice.next('ul').find('span.channel').each(function(){
					chlData = getChlFullInfo($(this));	 
					CloseWind($(this).attr('wind'),chlData.dev_id);
				})
			}else{
				oDevice.next('ul').find('span.channel').each(function(){
					chlData = getChlFullInfo($(this));	 
					if(!$(this).attr('wind')){
						oDevice.attr('bAllopen','1');
						var windState = oPreView.GetWindowConnectionStatus(wind);
						var win = wind;
						if(windState != 2){
							win = getWind(wind);
						}
						openWind(win,chlData);
					}	
				})
			}
			
			if(oDevice.attr('bAllopen')){ 
				var str = getNowTime()+'   正在从当前点击的窗口'+(parseInt(wind)+1)+', 开始往后依次打开设备:'+chlData.name+'下的所有通道';
			}else{ 
				var str = getNowTime()+'   正在关闭设备:'+chlData.name;
			}
			writeActionLog(str);
		})

		//显示分屏的文字
		$('div.operat li.setViewNum').click(function(){ 
			setViewNumNow();
		})

		//日志区域右键从菜单
		$('#actionLog').mouseup(function(){
			if(event.which == 3){
				var l = event.pageX > $(this).width() - 60 ? $(this).width() - 60 : event.pageX;
				var t = event.pageY - $(this).offset().top 
					t = t > $(this).height() - 19 ? $(this).height() - 19: t;
				$(this).find('a.emptyAct').css({ 
					left:l,
					top:t
				}).show();
				$(document).click(function(){ 
					$(this).find('a.emptyAct').hide();
					$(document).off();
				})
			}
		})

		//同步设置分屏UI
		var indexLi = $('li.setViewNum[onclick*='+oCommonLibrary.getSplitScreenMode()+']'),
			backPosition = indexLi.css('background-position').split(' ');
			indexLi.css('background-position','-30px '+backPosition[1]);

		$('#setModel').css('background-position',indexLi.css('background-position'));

		setViewNumNow();
		//绑定控件事件
		oPreView.AddEventProc('CurrentWindows','WindCallback(ev)')

		oPreView.AddEventProc('CurrentStateChange','windChangeCallback(ev)');
		var url =['index.html','play_back.html','backup.html','device.html','log.html']
		/*for(i in url){
			if(i != 0){ 
				$('#winCon')[0].LoadNewPage('/skins/default/'+url[i]);
			}
		}*/
		$('div.top_nav li').each(function(index){
			$(this).click(function(){ 
				if(index == 0){
					return false;
				}
				$('#winCon')[0].LoadNewPage('/skins/default/'+url[index]);
			})	
		})

		//window.status = '<pageaction SrcUrl="/skins/default/index.html" SrcAct="index" DstUrl="/skins/default/log.html" DstAct="reload"></pageaction>';
	})///

	function CloseWind(wind,dev_id){ 
		oPreView.CloseWndCamera(wind);
		writeActionLog('通道'+wind+'已经关闭!');
	}

	function openCloseAll(bool){  //打开关闭所有窗口
		if(bool){
			var wind = 0;

			$('div.dev_list:visible span.channel').not('[wind]').each(function(){
				wind = getWind(wind);
				openWind(wind,getChlFullInfo($(this)));
				wind++;
			})

			writeActionLog('正在打开当前列表下的所有通道');
		}else{

			$('div.dev_list:visible span.channel[wind]').each(function(){
				CloseWind($(this).attr('wind'),getChlFullInfo($(this)).dev_id);
			})

			writeActionLog('正在关闭当前列表下的所有通道');
		}
	}

	function checkAllchannelOpen(){
		var b = 1;
		$('div.dev_list:visible span.channel').each(function(){
			if(!$(this).attr('wind')){
				b = 0;
			}
		})
		var obj = $('#openAllchannel')
		if(b){
			obj.attr('toggle',1).css('background-position','0px'+' '+(-obj.height())+'px');
		}else{
			obj.removeAttr('toggle').css('background-position','0 0');
		}	
	}

	function openWind(wind,data){
		var windState = oPreView.GetWindowConnectionStatus(wind);
		if(windState != 2 ){ //该窗口不可用.
			var sWind = parseInt(wind)+1;
			var str = getNowTime()+'   设备:'+data.name+' 下的通道:'+data.channel_name+' 在窗口'+sWind+',打开失败！  错误:当前窗口'+sWind+' '+winState[windState];
			writeActionLog(str);
		}
		wind = getWind(wind);
		$('#channel_'+data.channel_id+',#g_channel_'+data.channel_id).attr('wind',wind);

		oPreView.SetDevChannelInfo(wind,data.channel_id);

		oPreView.OpenCameraInWnd(wind,data.address,data.port,data.eseeid,data.channel_number,data.stream_id,data.username,data.password,data.channel_name,data.vendor);
	}

	function WindCallback(ev){ 
		var obj = $('div.dev_list span.channel').filter(function(){ 
			return $(this).attr('wind') == ev.Wid;
		})
		$('div.dev_list span.channel').removeClass('sel');
		obj.addClass('sel');
	}

	function windChangeCallback(ev){ //CurrentState 0 STATUS_CONNECTED,1 STATUS_CONNECTING,2 STATUS_DISCONNECTED,3 STATUS_DISCONNECTING;
		var obj = $('#channel_'+ev.ChannelId);
		/*$('div.dev_list span.channel').filter(function(){ 
			return $(this).attr('wind') == ev.WPageId;
		})*/
		var chlData = getChlFullInfo(obj);
		var str=getNowTime()+'   设备:'+chlData.name+' 下的通道'+chlData.channel_name+'在窗口'+(parseInt(ev.WPageId)+1)+' '+currentWinStateChange[ev.CurrentState];
		if(ev.CurrentState == 2){			
			obj.removeAttr('state wind').removeClass('channel_1');
			checkDevAllOpen(obj.data('data').dev_id);
			//checkAllchannelOpen()
		}else if(ev.CurrentState == 0){	
			checkDevAllOpen(obj.data('data').dev_id);
			obj.addClass('channel_1');
			//checkAllchannelOpen()
		}else{
			str=''
			obj.attr({state:ev.CurrentState,wind:ev.WPageId});
		}
		writeActionLog(str);
	}
	//获取当前窗口最经一个可用的窗口。
	function getWind(i){
		if(oPreView.GetWindowConnectionStatus(i)!=2){
			i++;
			if(i>64){
				i=0;
			}
			return getWind(i);
		}else{ 
			return i;
		}
	}
	function checkDevAllOpen(dev_id){ 
		var bAllopen = 1;
		var oDev =$('#dev_'+dev_id);
		oDev.next('ul').find('span.channel').each(function(){ 
			if(!$(this).attr('wind')){ 
				bAllopen = 0;
			};
		})
		if(bAllopen ==1){
			oDev.attr('bAllopen',bAllopen);
			oDev.addClass('device_1');
		}else{ 
			oDev.removeAttr('bAllopen');
			oDev.removeClass('device_1');
		}
	}
	function setViewMod(i){
		oPreView.setDivMode(i);
	}
	function setViewNumNow(){     //显示当前分屏模式和当前第级分屏
		var str = (oPreView.getCurrentPage()+1)+'/'+oPreView.getPages();
		$('#nowWinMod').html('').html(str);
	}
	function preNextPage(type){ 
		if(type){ 
			oPreView.prePage();
		}else{ 
			oPreView.nextPage();
		}
		setViewNumNow();
	}
	function writeActionLog(str){ 
		if(str){
			$('<p>'+str+'</p>').prependTo('#actionLog');
		}
	}
	function showEmptyAction(){ 
		$('#actionLog a.emptyAct').show();
	}
	function emptyLog(){
		$('#actionLog p').remove();	
		$('#actionLog a.emptyAct').hide();
	}
	function getChlFullInfo(oChl){ 
		var dev_id = oChl.data('data').dev_id;
		var devData = $('#dev_'+dev_id).data('data');
		var chlData = oChl.data('data');
		for(i in devData){ 
			chlData[i]=devData[i];
		}
		return chlData;
	}
	function getNowTime(){
		var now = new Date();
		var H = now.getHours();
			H = H<10 ? '0'+H:H;
		var M = now.getMinutes();
			M = M<10 ? '0'+M:M;
		var S = now.getSeconds();
			S = S<10 ? '0'+S:S;
		return H+':'+M+':'+S;	
	}

	function Record(bool){
		if(bool){
			$('div.dev_list span.channel[wind]').each(function(){
				var data = $(this).data('data'),
					str = '';
				if(!oPreView.SetDevInfo(data.name,data.channel_number,$(this).attr('wind'))){
					if(!oPreView.StartRecord($(this).attr('wind'))){
						str = '设备'+data.name+' 下的通道'+data.channel_name+'开始录像!'	
					}else{ 
						str = '设备'+data.name+' 下的通道'+data.channel_name+'录像失败!'
					}
				}else{ 
					str = '设备'+data.name+' 下的通道'+data.channel_name+'的录像数据绑定失败!'
				}
				writeActionLog(str);
			})
		}else{
			$('div.dev_list span.channel[wind]').each(function(){
				var data = $(this).data('data'),
					str = '';
				if(!oPreView.StopRecord($(this).attr('wind'))){ 
					str = '设备'+data.name+' 下的通道'+data.channel_name+'关闭录像!'
				}else{ 	
					str = '设备'+data.name+' 下的通道'+data.channel_name+'关闭录像失败!'
				}
				writeActionLog(str);
			})
		}
	}
