var oLeft,oBottom,oView,oPreView;
var	nViewNum = 0;
var timer = null;
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
				/*var i =0;
				setInterval(function(){
					alert(123);
					i++;
					show(i);
				},1000);*/
			
				if($(this).attr('state')){
					CloseWind($(this).attr('wind'));
				}else{
					openWind($(this));
				}
			})
		})
		$('div.dev_list span.device').each(function(){ 
			var oDevice = $(this);
			oDevice.attr('bAllopen','1').click(function(){
				oDevice.next('ul').find('span.channel').each(function(){
					if(!$(this).attr('wind')){
						oDevice.attr('bAllopen','0')
						openWind($(this));
					}
					if(oDevice.attr('bAllopen')){ 
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
	function openWind(node){
		var devData = node.parent('li').parent('ul').prev('span.device').data('data');
		var chlData = node.data('data');
		var wind = oPreView.GetCurrentWnd()
		alert('焦点窗口为'+wind+'|状态为:'+oPreView.GetWindowConnectionStatus(wind));
		if(oPreView.GetWindowConnectionStatus(wind) != 2 ){ //该窗口不可用。
			wind = getWind(0);
		}
		for(i in chlData){ 
			devData[i]=chlData[i];
		}
		node.attr('wind',wind);
		alert('检测焦点窗口状态后调整的打开窗口为:'+wind);
		oPreView.OpenCameraInWnd(wind,devData.address,devData.port,devData.eseeid,chlData.channel_number,chlData.stream_id,devData.username,devData.password,chlData.channel_name,devData.vendor);
	}
	function WindCallback(ev){ 
		var obj = $('div.dev_list span.channel').filter(function(){ 
			return $(this).attr('wind') == ev.Wid;
		})
		$('div.dev_list span.channel').removeClass('sel');
		obj.addClass('sel');
	}
	function windChangeCallback(ev){ //CurrentState 0 STATUS_CONNECTED,1 STATUS_CONNECTING,2 STATUS_DISCONNECTED,3 STATUS_DISCONNECTING;
		clearInterval(timer);
		var obj = $('div.dev_list span').filter(function(){ 
			return $(this).attr('wind') == ev.WPageId;
		})
		if(ev.CurrentState == 2){
			obj.removeAttr('state wind');
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