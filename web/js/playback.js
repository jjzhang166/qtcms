var oLeft,oBottom,oView,oPlayBack,
	nViewNum = 0,
	NowMonth = 0,
	num = 0;
	$(function(){
		oLeft = $('#search_device');
		oBottom = $('#operating');
		oView = $('#playback_view')
		oPlayBack = $('#playback')[0];
		ViewMax();
		var oAs = $('ul.dev_list_btn a'),
			oDiv = $('div.dev_list');
	    
	    $('ul.filetree').treeview();		
		oDiv.eq(1).hide();
		
		$('.hover').each(function(){  // 按钮元素添加鼠标事件对应样式
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

		$('div.menu .close').click(function(){  //弹出操作框下部分元素添加关闭窗口事件
			closeMenu();
		})

		var oSelected = [];
		$('div.dev_list span.device').click(function(){
			$('div.dev_list span.device').removeClass('sel');
			$(this).addClass('sel');
			
			oSelected = [];
			
			var oDevData=$(this).data('data')
			show(oDevData);
			if(oPlayBack.setDeviceHostInfo(oDevData.address,oDevData.port,oDevData.eseeid)){ 
				alert('IP地址设置失败或者端口不合法!');
				return false;
			}
			if(oPlayBack.setDeviceVendor(oDevData.vendor)){
				alert('vendor为空设置失败!');
				return false;
			}

			var oVideoList = $("#channelvideo")
			var i=0;
			oVideoList.find('input:checkbox:checked').each(function(index){
				alert('设备'+oDevData.name+'下的通道'+index+'绑定到窗口'+i+',  状态为:'+oPlayBack.AddChannelIntoPlayGroup(i,index)) 
				if(oPlayBack.AddChannelIntoPlayGroup(i,index) != 0){
					alert('设备'+oDevData.name+'下的通道'+index+'绑定到窗口'+i+'失败!');
					return false;
				};
				i++;
				i = i > 4 ? 4 : i;
			});
			if(oVideoList.setUserVerifyInfo(oDevData.username,oDevData.password)){
				alert('设备'+oDevData.name+'下的通道'+index+'绑定到窗口'+i+'失败!');
				return;
			};
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

		$('div.play_time').mousedown(function(event){
			event.stopPropagation();
			var disX = event.pageX - $(this).offset().left;
			set_drag(disX,79,$('table.table').width() -42);
		})

		$("#channelvideo").mousedown(function(event){
			event.stopPropagation();
			$('div.play_time').css('left',event.pageX-2);
			set_drag(0,79,$('table.table').width() -42);
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

	})///
	function searchVideo(){
		var devData = $('div.dev_list span.device.sel').data('data');
		if(!devData){
			Confirm('请选择一台设备');
			return false;
		}
		$('#channelvideo div.video').remove();
		var channels = 0;
		$('#channelvideo input:checkbox').each(function(index){ 
			if($(this).is(':checked')){
				channels += 1 << index;
			}
		});
		var type = $('#type span').attr('type')
			type = type == 0 ? 15 : 1 << parseInt(type)-1;
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
		}
	}
	var color = [];
		color[1] = '#7BC345';
		color[2] = '#FFE62E';
		color[4] = '#F00';
		color[8] = '#F78445';
	function VideoData2Ui(obj){
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
	function gettime(objs){
		var time = []
		objs.each(function(){ 
			time.push($(this).val());
		})
		return time.join(':');
	}