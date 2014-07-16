var oCommonLibrary,
	key=0; //当前填充设备列表的索引;
	$(function(){ 
		//return false;
		oCommonLibrary = document.getElementById('commonLibrary');

		$('ul.filetree').treeview();
		//多语言提示转换
		$('li[title],div[title],a[title],span[title]').each(function(){
			$(this).attr('title',lang[$(this).attr('title')])
		})
		_t($('input:text'));
	})
	function refresh(data){
		
		if(data.Dsturl.indexOf('play_back') != -1){
			
			areaList2Ui();
			
			$('div.dev_list span.device').each(function(){
				$(this).parent('li').on({
					dblclick:function(){ //设备双击开始搜索
						playBackSerchFile();
					},
					click:function(){ //单击同步选中状态
						$('div.dev_list li,span').removeClass('sel');
						$(this).addClass('sel');
					}
				})
			})

			/*if(recFile)
				loclFileDataIntoChannel(recFile);*/
		}
		window.initOxcDevListStatus();
		//区域列表;

		/*if(data.Dsturl == 'null'){
			$('span.channel').removeClass('channel_1');
			for(var i=0;i<64;i++){
				var oWinInfo = oPreView.GetWindowInfo(i);
				if(oWinInfo.chlId!=-1 && oWinInfo.currentState == 0){
					var chlData = $('#channel_'+oWinInfo.chlId).attr({
						wind:i,
						state:oWinInfo.currentState
					}).addClass('channel_1').data('data');
					checkDevAllOpen(chlData.dev_id);
				}
			}	
		}else if(data.Dsturl.indexOf('backup') != -1){
			
			areaList2Ui(0,1);
			initrecFileOcx($('tbody.search_result tr'))
			
		}else if(data.Dsturl.indexOf('play_back') != -1){
			initrecFileOcx($('#channelvideo div.video'));
			dragStopMove();
			palybackspeed('1X');

		}else if(data.Dsturl.indexOf('device') != -1){
			//分组列表;
			groupList2Ui();
			areaList2Ui(key);

			set_contentMax();

			searchEdDev();

			if(key == 0){
				searchFlush();
			}else{
				oSearchOcx.Stop();
			}
		}*/
	}
	
	//区域分组,属性菜单输出.
	function areaList2Ui(num,bool,closed){ //区域菜单输出
		num = num || key ;
		if(num != 0){
			closed='';
		}
		var obj = $('ul.filetree').not('[id]');

			obj.each(function(){
				$(this).treeview({remove:$(this).find('li:first')});	
			})
			obj = obj.eq(num);

		var add = $('<li><span class="area" id="area_0">'+lang.Area+'</span><ul></ul</li>').find('span.area:first').data('data',{'area_id':'0','area_name':lang.Area,'pid':'0','pareaname':'root'})
				  .end().appendTo(obj);

		obj.treeview({add:add});

		var areaListArrar=[];
		var pidList=[];
		var areaList = oCommonLibrary.GetAreaList();
		for(n in areaList){
			var id = areaList[n];
			var name = oCommonLibrary.GetAreaName(areaList[n]);
			var pid = oCommonLibrary.GetAreaPid(areaList[n]);
			var pareaname = pid == 0 ? lang.Area : oCommonLibrary.GetAreaName(pid);
			areaListArrar.push({'area_id':id,'pid':pid,'area_name':name,'pareaname':pareaname});
			pidList.push(pid);

		}
		var arr =del(pidList.sort(sortNumber)); //  返回pid升序的PID数组
		deviceList2Ui('0',num,bool,closed);
		for(j in arr){
			for(k in areaListArrar){		
				if(areaListArrar[k]['pid'] == arr[j]){		
					var add = $('<li><span class="area" id="area_'+areaListArrar[k]['area_id']+'">'+areaListArrar[k]['area_name']+'</span><ul></ul></li>').appendTo($('#area_'+arr[j]).next('ul'));
					add.find('span.area').data('data',areaListArrar[k]);
					obj.treeview({add:add});
					deviceList2Ui(areaListArrar[k]['area_id'],num,bool);
				}
			}
		}
	}
	function deviceList2Ui(areaid,num,bool,closed){ //设备菜单输出
		bool = num == 1 ? true : bool;
		var devList = oCommonLibrary.GetDeviceList(areaid);
		for (i in devList){
			var id=devList[i];
			var devData = oCommonLibrary.GetDeviceInfo(id);
			devData['area_id'] = areaid;
			devData['dev_id'] = id;
			devData['channel_count'] = oCommonLibrary.GetChannelCount(id);
			devData['device_name'] = devData['name'];
			devData['eseeid'] = devData['eseeid'];
			devData['parea_name'] = oCommonLibrary.GetAreaName(areaid) || lang.Area;
			var add = $('<li class="'+closed+'"><span class="device" id="dev_'+id+'" >'+devData['name']+'</span>'+(bool ? '':'<ul></ul>')+'</li>').appendTo($('#area_'+areaid).next('ul'));
			add.find('span.device').data('data',devData);
			$('ul.filetree:eq('+num+')').treeview({add:add});
			devChannelList2Ui(id,num);
		}
	}
	function devChannelList2Ui(devid,num){ //设备下通道菜单输出
		var chlList = oCommonLibrary.GetChannelList(devid);
		//console.log(chlList);	
		for(i in chlList){ 
			var id = chlList[i];
			//if(!$('#channel_'+id)[0]){ 
				var chldata = oCommonLibrary.GetChannelInfo(id);
				var data = {};
				data['channel_id'] = id;
				data['dev_id'] = devid;
				data['channel_number'] = chldata['number']
				data['stream_id'] = chldata['stream'];
				data['channel_name'] = chldata['name'];
				//show(data);
				var add = $('<li><span class="channel" id="channel_'+id+'" >'+chldata['name']+'</span></li>').appendTo($('#dev_'+devid).next('ul'));
				add.find('span.channel').data('data',data);
				$('ul.filetree:eq('+num+')').treeview({add:add});
			/*}else{ 
				$('#channel_'+id).data('data')['dev_id'] = devid;
			}*/	
		}
	}
	function groupList2Ui(){   //分组菜单输出
		var obj = $('#group_0')
		$('ul.filetree:eq(1)').treeview({remove:obj.find('li')})
		if(!obj[0]){
			return;
		}
		var groupList = oCommonLibrary.GetGroupList();
		$('#group_0').data('data',{'group_id':'0','group_name':lang.Grouping,'pid':'0','pareaname':'root'}).html('');
		for( i in groupList){
			var id = groupList[i];
			var name =oCommonLibrary.GetGroupName(id);
			var add = $('<li class="closed"><span class="group" id="group_'+id+'">'+name+'</span><ul></ul></li>').appendTo(obj);
			add.find('span.group').data('data',{'group_id':id,'group_name':name});
			obj.treeview({add:add});
			groupChannelList2Ui(id);
		}
	}
	function groupChannelList2Ui(groupId){  //分组下通道菜单输出
		var chlList = oCommonLibrary.GetGroupChannelList(groupId);
		for(i in chlList){ 
			var id = chlList[i];
			var chldata = oCommonLibrary.GetChannelInfoFromGroup(id);
			var data = {};
			data['r_chl_group_id'] = id;
			data['channel_id'] = oCommonLibrary.GetChannelIdFromGroup(id);
			var chldata2 = oCommonLibrary.GetChannelInfo(data['channel_id']);
			data['channel_number'] = chldata2['number'];
			data['stream_id'] = chldata2['stream'];
			data['dev_id'] = chldata2['dev_id'];
			data['channel_name'] = chldata['name'];
			data['group_id'] = chldata['group_id'];
			data['r_chl_group_name'] = ['name'];
			var add = $('<li><span class="channel" id="g_channel_'+data['channel_id']+'" >'+chldata['name']+'</span></li>').appendTo($('#group_'+groupId).next('ul'));
			add.find('span.channel').data('data',data);
			$('ul.filetree:eq(1)').treeview({add:add});
		}
	}