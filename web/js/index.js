var oLeft,oBottom,oView,oPreView;
var	nViewNum = 0;
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
					oPreView.CloseWndCamera($(this).attr('wind'));
					$(this).attr({state:'',wind:''});
				}else{
					var devData = $(this).parent('li').parent('ul').prev('span.device').data('data');
					var chlData = $(this).data('data');
					var wind = oPreView.GetCurrentWnd()
					if(oPreView.GetWindowConnectionStatus(wind) !=0){ 
						wind = getWind(0);
					}
					for(i in chlData){ 
						devData[i]=chlData[i];
					}
					/*oPreView.OpenCameraInWnd(wind,devData.address,devData.port,devData.eseeid,chlData.channel_number,chlData.stream_id,devData.username,devData.password,chlData.channel_name,devData.vendor);*/
					oPreView.SetCameraInWnd(wind,devData.address,devData.port,devData.eseeid,chlData.channel_number,chlData.stream_id,devData.username,devData.password,chlData.channel_name,devData.vendor);
					$(this).attr({state:'1',wind:wind});
				}
			})
		})
		$('div.operat li.setViewNum').click(function(){ 
			setViewNumNow();
		})
		setViewNumNow();
	})
	//获取当前窗口最经一个可用的窗口。
	function getWind(i){
		if(oPreView.GetWindowConnectionStatus(i)!=0){
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
	function oxcAddEvent(){
		oPreView.attachEvent("CurrentStateChange",function a(CurrentState,WPageId){
			var node = $('div.dev_list span.channel').filter(function(){ 
				return $(this).attr('wind') == WPageId;
			})
			node.attr('state',CurrentState);
			if(CurrentState == 1){ 
				//node.addClass('open_chl').prev('span').html(turn_off);
				//checkIsAllChannlOpen(node.parent('li').data('data')[0]);
			}else if(CurrentState == 2 || CurrentState == 4 || CurrentState == 6){
				/*if(CurrentState == 2){ 
					node.addClass('open_fail');
				}
				if(node.length != 0){
					checkIsAllChannlOpen(node.parent('li').data('data')[0]);
				}
				node.removeAttr('wind').removeClass('open_chl').prev('span').html(turn_on);*/
			}
			/*CurrentState:当前状态
			0-正在连接
			1-连接成功
			2-连接失败
			3-正在关闭
			4-关闭成功
			5-关闭失败
			6-断开连接
			nExtensionData:附带数据
				当CurrentState为0时，数据
					0表示:bubble协议连接
					1表示:穿透连接
					2表示:转发连接*/
			/*switch(CurrentState){ 
				case 0:		
					node.attr('state',CurrentState);	
				break;
				case 1:
					node.attr('state',CurrentState).addClass('open_chl').next('span').html('关闭');
					checkIsAllChannlOpen(node);
				break;
				case 2:
					node.attr('state',CurrentState).removeAttr('wind').next('span').html('打开')
				break;
				case 3:
					node.attr('state',CurrentState);
				break;
				case 4:
					node.attr('state',CurrentState).removeAttr('wind').removeClass('open_chl').next('span').html('打开');
					checkIsAllChannlOpen(node);
				break;
				case 5:
					node.attr('state',CurrentState);
				break;
				default:
				break;
			}*/
		})
	}