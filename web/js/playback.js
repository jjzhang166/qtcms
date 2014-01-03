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
			//show(oDevData);
			if(!setDevData2ocx(oDevData)){ 
				return false;
			}

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

		$('div.play_time').on({ 
			mousedown: function(event){
				event.stopPropagation();
				oPlayBack.GroupStop();
				var disX = event.pageX - $(this).offset().left;
				set_drag(disX,79,$('table.table').width() -42);
			},
			mouseup:function(event){
				event.stopPropagation();
				$(document).off();
				playVideo(event.pageX);
			}
		})

		$("#channelvideo").on({ 
			mousedown:function(event){
				event.stopPropagation();
				$('div.play_time').css('left',event.pageX-2);
				set_drag(0,79,$('table.table').width() -42);
			},
			mouseup:function(event){ 
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
	})///

	function searchVideo(){
		var devData = $('div.dev_list span.device.sel').data('data');
		if(!devData){
			alert('请选择一台设备');
			return false;
		}
		$('#channelvideo div.video').remove();
		var channels = 0;
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
		}
		ocxsearchVideo(devData);
	}
	function playVideo(pageX){ 
		var disX = pageX - $('div.play_time').offset().left,
			X1 = 79,
			X2 = $('table.table').width() -42,
			left = pageX - disX,
			date = $("div.calendar span.nowDate").html(),
			sScond = parseInt(((left-X1)/(X2-X1)*24*3600)),
			type = parseInt($('#type span').attr('type')),
			begin = date+' '+returnTime(sScond),
		end = date+' 23:59:59';
		type = type == 0 ? 15 : 1 << type;
		oPlayBack.GroupPlay(type,begin,end);
	}
	function setDevData2ocx(oDevData){
		var  b = true;
		if(oPlayBack.setDeviceHostInfo(oDevData.address,oDevData.port,oDevData.eseeid)){ 
			alert('IP地址设置失败或者端口不合法!');
			b = false;
		}
		if(oPlayBack.setDeviceVendor(oDevData.vendor)){
			alert('vendor为空设置失败!');
			b = false;
		}
		$("#channelvideo").find('input:checkbox').each(function(index){
			if($(this).is(':checked')){
				if(oPlayBack.AddChannelIntoPlayGroup(index,(index+1))){
					b = false;
				};
			}
		});
		oPlayBack.setUserVerifyInfo(oDevData.username,oDevData.password);
		return b
	}

	var typeHint = [];
		typeHint[1] = '定时';
		typeHint[2] = '运动';
		typeHint[4] = '警告';
		typeHint[8] = '手动';
		typeHint[15] = '全部';
	function ocxsearchVideo(devData){
		var type = $('#type span').attr('type');
			type = type == 0 ? 15 : 1 << type;
		var date = $("div.calendar span.nowDate").html();
		var startTime =date+' '+gettime($('div.timeInput:eq(0) input'));
		var endTime =date+' '+gettime($('div.timeInput:eq(1) input'));
		$("#channelvideo").find('input:checkbox').each(function(index){
			if($(this).is(':checked')){
				if(oPlayBack.startSearchRecFile(index,type,startTime,endTime)!=0){
						alert('控件检索设备'+devData.name+'下的通道'+index+'的'+typeHint[type]+'录像失败');
				};
			}
		});
	}

	var color = [];
		color[1] = '#7BC345';
		color[2] = '#FFE62E';
		color[4] = '#F00';
		color[8] = '#F78445';
		color[15] = '#ABCDEF';
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