var oLeft,oBottom,oView,oPreView;
var	nViewNum = 0;
var timer = null;
var winState=['已经接入了连接!','正在连接!','断开连接!','正在断开连接!'];
	$(function(){
		oLeft = $('#search_device');
		oBottom = $('#operating');
		oView = $('#playback_view');
		oPreView= $('#previewWindows')[0];
		setViewMod('div2_2')

		var oAs = $('ul.dev_list_btn a');
		var oDiv = $('div.dev_list');
	    
		$(window).off();

	    $('ul.filetree').treeview().find('span.file').click(function(){
			$(this).toggleClass('file_1')
		});
		oDiv.eq(1).hide();

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
		
		ViewMax('preview');
		
		$('body')[0].onresize=function(){
			ViewMax('preview');
		}
 
		$('div.dev_list span.channel').each(function(){ 
			$(this).click(function(){		
				if($(this).attr('state')){
					CloseWind($(this).attr('wind'));
				}else{
					var chlData = getChlFullInfo($(this));
					openWind(oPreView.GetCurrentWnd(),chlData);
				}
			})
		})
		$('div.dev_list span.device').each(function(){ 
			var oDevice = $(this);
			oDevice.attr('bAllopen','1').click(function(){
				oDevice.next('ul').find('span.channel').each(function(){
					if(!$(this).attr('wind')){
						oDevice.attr('bAllopen','0')
						var wind = oPreView.GetCurrentWnd();
						var windState = oPreView.GetWindowConnectionStatus(wind);
						if(windState != 2){
							wind = getWind(wind);
						}
						var chlData = getChlFullInfo($(this));
						openWind(wind,chlData);
					}
					if(oDevice.attr('bAllopen') == 1){ 
						CloseWind($(this).attr('wind'));
					}
				})

			})
		})
		$('div.operat li.setViewNum').click(function(){ 
			setViewNumNow();
		})
		setViewNumNow();

		oPreView.AddEventProc('CurrentWindows','WindCallback(ev)')

		oPreView.AddEventProc('CurrentStateChange','windChangeCallback(ev)')
	})///
	function CloseWind(wind){ 
		oPreView.CloseWndCamera(wind);

	}
	function openWind(wind,data){	
		var windState = oPreView.GetWindowConnectionStatus(wind)
		if(windState != 2 ){ //该窗口不可用.
			var str = '设备:'+data.name+' 下的通道:'+data.channel_name+' 在窗口'+wind+',打开失败！  错误:当前窗口'+wind+' '+winState[windState];
			writeActionLog(str);
			return;
		}
		$('#channel_'+data.channel_id).attr('wind',wind);
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
		var obj = $('div.dev_list span').filter(function(){ 
			return $(this).attr('wind') == ev.WPageId;
		})
		if(ev.CurrentState == 2){
			obj.removeAttr('state wind');
		}else if(ev.CurrentState == 0){ 	
			checkDevAllOpen(obj.data('data').dev_id);
		}else{
			obj.attr({state:ev.CurrentState,wind:ev.WPageId});
		}
	}
	//获取当前窗口最经一个可用的窗口。
	function getWind(i){
		if(oPreView.GetWindowConnectionStatus(i)!=2){
			i++;
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
		oDev.attr('bAllopen',bAllopen);

	}
	function setViewMod(i){
		oPreView.setDivMode(i);
	}
	function setViewNumNow(){     //显示当前分屏模式和当前第级分屏
		var str = oPreView.getCurrentPage()+'/'+oPreView.getPages();
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
		$('<p>'+str+'</p>').appendTo('#actionLog');
	}
	function emptyLog(){
		$('#actionLog p').remove();	
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