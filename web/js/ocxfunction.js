
var oCommonLibrary,
	bool = 0, //本地远程回放控制  0为远程 1为本地
	recTotal = 0;  //文件检索总数。
	$(function(){ 
		//return false;
		oCommonLibrary = document.getElementById('commonLibrary');
		//分组列表;
		groupList2Ui();
		//区域列表;
		areaList2Ui('0');
	})
	function refresh(data){ 
		//分组列表;
		groupList2Ui();
		//区域列表;
		areaList2Ui('0');
	}
	var userLev = ['超级管理员','管理员','用户','游客'];
	
	function AddUserSuccess(ev){
		var name =$('#username_add_ID').val();
		var userCom= $('#level_add_ID').prev('a').prev('span').html();
		for(i in userLev){
			if(userLev[i] == userCom){
				var userlv = i;
			}
		}
		var No = $('#UserMan table.UserMan tbody tr').length - 1;
		var data = {'username':name,'userlv':userlv,'userCom':userCom}
		$('<tr><td>'+No+'</td><td>'+name+'</td><td>'+userCom+'</td></tr>').appendTo('#UserMan table.UserMan').data('data',data);
		closeMenu();	
	}
	function DeleteUserSuccess(){
		$('#username_list_ID').val('');
		$('#UserMan tr.selected').remove();
	}

	function ModifyUserSuccess(ev){
		var userCom= $('#level_modify_ID').prev('a').prev('span').html();
		for(i in userLev){
			if(userLev[i] == userCom){
				var userlv = i
			}
		}
		var name = $('#username_modify_ID').val();
		var data = {'username':name,'userlv':userlv,'userCom':userCom}
		$('#UserMan tr.selected:first').data('data',data).find('td:last').html(userCom);
		closeMenu();
	}

	function Fail(data){
		var str='';
		if(data.name){
			str +=data.name+': ';
		}else if(data.channelname){ 
			str +=data.name+': ';
		}
		str += data.fail
		Confirm(str);
	}

	//搜索设备控件方法.
	function searchFlush(){
		oSearchOcx.Stop();
		$('#SerachDevList tbody tr').remove();	 
		oSearchOcx.Start();
		/*setTimeout(function(){
			oSearchOcx.Stop();
		},5000)*/
	}
	//设备搜索回调函数
	function callback(data){
		var bUsed = true;
		$('#SerachDevList tbody tr').each(function(){ 
			if(parseInt($(this).find('td:eq(1)').html()) == data.SearchSeeId_ID || $(this).find('td:eq(2)').html() == data.SearchIP_ID){
				bUsed = false;
			}
		})
		
		$('div.dev_list:eq(0) span.device').each(function(){ 
			if($(this).data('data')['eseeid'] == data.SearchSeeId_ID){ 
				bUsed = false;
			}
		})

		if(bUsed){
			var id = data.SearchSeeId_ID > 1 ? data.SearchSeeId_ID : data.SearchIP_ID.replace(/\./g,'-');
			var sClass = data.SearchVendor_ID.split(' ')[1];
			$('<tr id="esee_'+id+'" class="'+sClass+'"><td><input type="checkbox" />'+data.SearchVendor_ID+'</td><td>'+data.SearchSeeId_ID+'</td><td>'+data.SearchIP_ID+'</td><td>'+data.SearchChannelCount_ID+'</td></tr>').appendTo($('#SerachDevList tbody')).data('data',data);
			initDevIntoAreaXml($('#SerachDevList tbody input:checkbox'),$('#adddevicedouble_ID'));
		}
				
	}
	function AddAreaSuccess(data){
		var name = $('#area_name_ID').val();
		var pid = $('#pid_ID').val();
		var pidname = oCommonLibrary.GetAreaName(pid);
		var id = data.areaid;
		var add = $('<li><span class="area" id="area_'+id+'">'+name+'</span><ul></ul></li>').appendTo($('div.dev_list:eq(0) #area_'+pid).next('ul'));
		add.find('span.area').data('data',{'area_name':name,'pareaname':pidname,'area_id':id,'pid':pid});
		$('ul.filetree:eq(0)').treeview({add:add});
		closeMenu();
	} 

	function RemoveAreaSuccess(){
		var id = $('#confirm input:hidden').val();
		var devList = [];
		if(id != 0){
			$('#area_'+id).next('ul').find('span.device').each(function(){
				devList.push($(this).data('data')['dev_id']);
			});
			$('div.dev_list').find('#area_'+id).parent('li').remove();
		}
		for(i in devList){
			$('ul.filetree:eq(1) span.channel').filter(function(){ 
				return $(this).data('data')['dev_id'] == devList[i];
			}).parent('li').remove();
		}
		$('ul.filetree').treeview();
		closeMenu();
	}

	function ModifyAreaSuccess(){
		var oArea = $('#area_'+$('#area_id_ID').val());
		var sNewName = $('#area_name_ID').val()
		oArea.html(sNewName);
		oArea.data('data')['area_name']=sNewName;
		closeMenu();
	} 
	function AddGroupSuccess(data){
		var name = $('#group_name_ID').val();
		var id = data.groupid;
		var add = $('<li><span class="group" id="group_'+id+'">'+name+'</span><ul></ul></li>').appendTo('#group_0');
			add.find('span.group').data('data',{'group_id':id,'group_name':name});
			$('ul.filetree:eq(1)').treeview({add:add});
		closeMenu();
	}
	function RemoveGroupSuccess(data){ 
		var id=$('#group_id_ID').val();
		/*$('#group_'+id).next('ul').find('span.channel').each(function(){ 
			var devid = $(this).data('data')['dev_id'];
			var add = $(this).parent('li').appendTo($('div.dev_list:eq(0) #dev_'+devid).next('ul'));
			$('ul.filetree').treeview({add:add});
		})*/
		$('#group_'+id).parent('li').remove();
		$('ul.filetree').treeview();
		closeMenu();
	}
	function ModifyGroupSuccess(data){
		var id=$('#group_id_ID').val();
		var name = $('#group_name_ID').val();
		$('#group_'+id).html(name);
		$('#group_'+id).data('data')['group_name'] = name;
		closeMenu();
	}

	function ModifyChannelSuccess() {
		var id= $('#channel_id_ID').val();
		var oChannel=$('#channel_'+id)
		var name =$('#channel_name_ID').val();
		oChannel.html(name);
		oChannel.data('data')['channel_name'] = name;
		closeMenu();
	}
	function AddDeviceSuccess(data){  //单个添加设备.. 菜单添加设备
		var dataIndex={'area_id':'','address':'','port':'','http':'','eseeid':'','username':'','password':'','device_name':'','channel_count':'','connect_method':'','vendor':'','dev_id':data.deviceid,'parea_name':$('#parea_name_ID').val()}
		for(i in dataIndex){ 
			if(dataIndex[i] == ''){
				dataIndex[i] = $('#device #'+i+'_ID').val();
			}
		}
		adddev(dataIndex);	
	}
	function AddDeviceAllSuccess(data){
		AddDeviceDoubleSuccess(data);
	}
	function ModifyGroupChannelNameSuccess(data){
		var id= $('#channel_id_ID').val();
		var oChannel=$('#g_channel_'+id)
		var name =$('#channel_name_ID').val();
		oChannel.html(name);
		oChannel.data('data')['channel_name'] = name;
		closeMenu();
	}
	function ModifyDeviceSuccess(){
		var dataIndex={'area_id':'','address':'','port':'','http':'','eseeid':'','username':'','password':'','device_name':'','channel_count':'','connect_method':'','vendor':'','dev_id':'','parea_name':$('#parea_name_ID').val()}
		for(i in dataIndex){ 
			dataIndex[i] = $('#'+i+'_ID').val();
		}
		$('#dev_'+dataIndex.dev_id).data('data',dataIndex).html(dataIndex.device_name);
		closeMenu();
	}
	function RemoveDeviceSuccess(){ 
		var id = $('#confirm #dev_id_ID').val();
		$('div.dev_list:eq(0) #dev_'+id).parent('li').remove();
		$('ul.filetree:eq(1) span.channel').filter(function(){ 
			return $(this).data('data')['dev_id'] == id;
		}).parent('li').remove();
		$('ul.filetree').treeview();
		closeMenu();
	}

	function RemoveDeviceAllSuccess(data){
		$('#dev_'+data.deviceid).parent('li').remove();
		$('ul.filetree').treeview();
	}

	function AddDeviceDoubleSuccess(data){  //添加多个设备
		data.name = data.name.replace(/\./g,'-');  // 用设备名做ID 名字中的.号转换
		var area = $('div.dev_list:eq(0) span.sel:eq(0)').hasClass('area') ? $('div.dev_list:eq(0) span.sel:eq(0)') : $('div.dev_list:eq(0) span.area:eq(0)');
		var devData = $('#esee_'+data.name).data('data');
		var devData2={'area_id':area.data('data')['area_id'],'address':devData['SearchIP_ID'],'port':devData['SearchHttpport_ID'],'http':devData['SearchHttpport_ID'],'eseeid':data.name,'username':'admin','password':'','device_name':data.name,'channel_count':devData['SearchChannelCount_ID'],'connect_method':'0','vendor':devData['SearchVendor_ID'],'dev_id':data.deviceid,'parea_name':area.data('data')['area_name']};
		adddev(devData2);
		$('#esee_'+data.name).remove();
		$('#adddevicedouble_ID').val('');
	}

	function adddev(data){
		data.device_name = data.device_name.replace(/-/g,'.');  // 用设备名做ID 名字中的.号转换
		//Confirm(data.device_name+'AddSuccess!');
		var add = $('<li><span class="device" id="dev_'+data.dev_id+'" >'+data.device_name+'</span><ul></ul></li>').appendTo($('#area_'+data.area_id).next('ul'));
		add.find('span.device').data('data',data);
		var chlList = oCommonLibrary.GetChannelList(data.dev_id);
		for(i in chlList){
			var chldata = oCommonLibrary.GetChannelInfo(chlList[i]);
			var chlNum = parseInt(chldata.number)+1;
			var name = 'chl_'+chlNum;
			var chldata={'channel_id':chlList[i],'dev_id':data.dev_id,'channel_number':chlNum,'channel_name':name,'stream_id':'0'};
			var addchl = $('<li><span class="channel" id="channel_'+chlList[i]+'">'+name+'</span></li>').appendTo($('#dev_'+data.dev_id).next('ul'));
			addchl.find('span.channel').data('data',chldata);
			$('ul.filetree:eq(0)').treeview({add:addchl});
		}
		$('ul.filetree:eq(0)').treeview({add:add});
		closeMenu();
	}
	function AddChannelDoubleInGroupSuccess(data){
		data.channelname=data.channelname.replace(/-/g,'.');
		var group = $('div.dev_list:eq(1) span.sel:eq(0)').hasClass('group') ? $('div.dev_list:eq(1) span.sel:eq(0)') : $('div.dev_list:eq(1) span.group:eq(0)');
		if($('#g_channel_'+data.chlid)[0]){
			return false;
		}
			var add = $('div.dev_list:eq(0) #channel_'+data.chlid).parent('li').clone(true)
			    .find('span.channel')
			    .attr('id','g_channel_'+data.chlid)
			    .html(data.channelname)
			    .end()
			    .appendTo(group.next('ul'));
			add.find('span.channel').data('data')['r_chl_group_id'] = data.chlgroupid;
			add.find('span.channel').data('data')['channel_name'] =data.channelname;
		$('ul.filetree').treeview({add:add}).treeview().find('span.channel').removeClass('sel');
	}
	function RemoveChannelFromGroupSuccess(data){ 
		$('div.dev_list:eq(1) span.channel.sel').each(function(){
			$(this).parent('li').remove();//appendTo($('#dev_'+$(this).data('data').dev_id).next('ul'));
		})
		$('ul.filetree').treeview();
	}
	//区域分组,属性菜单输出.
	function areaList2Ui(num){ //区域菜单输出
		$('div.dev_list span.area').parent('li').remove();
		var obj = $('ul.filetree:eq('+num+')')
		var add = $('<li><span class="area" id="area_0">区域_root</span><ul></ul</li>').appendTo(obj);
		obj.treeview({add:add});
		add.find('span.area:first').data('data',{'area_id':'0','area_name':'区域_root','pid':'0','pareaname':'root'});
		var areaListArrar=[];
		var pidList=[];
		var areaList = oCommonLibrary.GetAreaList();
		for(n in areaList){
			var id = areaList[n];
			var name = oCommonLibrary.GetAreaName(areaList[n]);
			var pid = oCommonLibrary.GetAreaPid(areaList[n]);
			var pareaname = pid == 0 ? '区域_root' : oCommonLibrary.GetAreaName(pid);
			areaListArrar.push({'area_id':id,'pid':pid,'area_name':name,'pareaname':pareaname});
			pidList.push(pid);

		}
		var arr =del(pidList.sort(sortNumber)); //  返回pid升序的PID数组
		deviceList2Ui('0',num);
		for(j in arr){
			for(k in areaListArrar){		
				if(areaListArrar[k]['pid'] == arr[j]){		
					var add = $('<li><span class="area" id="area_'+areaListArrar[k]['area_id']+'">'+areaListArrar[k]['area_name']+'</span><ul></ul></li>').appendTo($('#area_'+arr[j]).next('ul'));
					add.find('span.area').data('data',areaListArrar[k]);
					obj.treeview({add:add});
					deviceList2Ui(areaListArrar[k]['area_id'],num);
				}
			}
		}
	}
	
	function deviceList2Ui(areaid,num){ //设备菜单输出
		var devList = oCommonLibrary.GetDeviceList(areaid);
		for (i in devList){
			var id=devList[i];
			var devData = oCommonLibrary.GetDeviceInfo(id);
			devData['area_id'] = areaid;
			devData['dev_id'] = id;
			devData['channel_count'] = oCommonLibrary.GetChannelCount(id);
			devData['device_name'] = devData['name'];
			devData['eseeid'] = devData['eseeid'];
			devData['parea_name'] = oCommonLibrary.GetAreaName(areaid) || '区域_root';
			var add = $('<li><span class="device" id="dev_'+id+'" >'+devData['name']+'</span><ul></ul></li>').appendTo($('#area_'+areaid).next('ul'));
			add.find('span.device').data('data',devData);
			$('ul.filetree:eq('+num+')').treeview({add:add});	
			devChannelList2Ui(id,num);
		}
	}
	function devChannelList2Ui(devid,num){ //设备下通道菜单输出
		var chlList = oCommonLibrary.GetChannelList(devid);
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
		var groupList = oCommonLibrary.GetGroupList();
		$('#group_0').data('data',{'group_id':'0','group_name':'分组_root','pid':'0','pareaname':'root'}).html('');
		for( i in groupList){
			var id = groupList[i];
			var name =oCommonLibrary.GetGroupName(id);
			var add = $('<li><span class="group" id="group_'+id+'">'+name+'</span><ul></ul></li>').appendTo('#group_0');
			add.find('span.group').data('data',{'group_id':id,'group_name':name});
			$('ul.filetree:eq(1)').treeview({add:add});
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
			data['channel_number'] = chldata2['number']
			data['stream_id'] = chldata2['stream'];
			data['dev_id'] = chldata2['dev_id'];
			data['channel_name'] = chldata['name'];
			data['group_id'] = chldata['group_id']
			data['r_chl_group_name'] = ['name'];
			var add = $('<li><span class="channel" id="g_channel_'+data['channel_id']+'" >'+chldata['name']+'</span></li>').appendTo($('#group_'+groupId).next('ul'));
			add.find('span.channel').data('data',data);
			$('ul.filetree:eq(1)').treeview({add:add});
		}
	}
	function SettingCommonParmSuccess(data){
		//alert(data);
	}
	function SettingRecordDoubleTimeParm(data){ //清空回放时间表单的数据
		$('#recordtime div.timeInput input').val('');
		$('#recordtime input:checkbox').prop('checked',false);
		$('ul.week.option li').removeData();
		$('#week').html('星期一').attr('value','0');
		$('#recordtime td.copyTo').find('a,span').attr('value','').not('a.all').html('');
	}
	function SettingStorageParmSuccess(data){
		//alert(data);
	}
	//搜索远程录像
function setDevData2ocx(){
		var oDevData = $('#dev_'+$('div.dev_list li.sel span.channel').data('data').dev_id).data('data');
		var b = true;
		try{
			palybackspeed('1x');
		}catch(e){ 
		
		}
		if(bool){
			if(oPlaybackLocl.SetSynGroupNum(4)){ 
				alert('同步组数量设置失败');
				b = false
			}
		}else{
			oPlayBack.GroupStop();
			oPlayBack.GroupSpeedNormal();
			if(oPlayBack.setDeviceHostInfo(oDevData.address,oDevData.port,oDevData.eseeid)){ 
				alert('IP地址设置失败或者端口不合法!');
				b = false;
			}

			if(oPlayBack.setDeviceVendor(oDevData.vendor)){
				alert('vendor为空设置失败!');
				b = false;
			}
			oPlayBack.setUserVerifyInfo(oDevData.username,oDevData.password);
			var intoWindsChl = $("#channelvideo").find('input:checkbox');
			if(intoWindsChl.length != 0){
				var i= 0;
				intoWindsChl.each(function(index){
					if($(this).is(':checked')){
						if(oPlayBack.AddChannelIntoPlayGroup(i,index)){
							b = false;
						};
						i++;
					}
				});
			}else{
				try{
					oPlaybackLocl.GroupStop();
					oPlaybackLocl.GroupSpeedNormal();
				}catch(e){}
				for(var i=0;i<oDevData.channel_count;i++){
					if(oPlayBack.AddChannelIntoPlayGroup(i,i)){
						b = false;
					}
				}
			}
		}
		return b;
	}
  	function berorSerchShowHint(obj) {
  		$('#fileRec').show().find('span').width(0)
					 .end().find('h5').html('0/0')
					 .end().find('h4').html('正在检索:');
		$('div.dev_list li').removeClass('sel');
		obj.addClass('sel');
  	}
	function searchVideo(){
		var seletDev = $('div.dev_list li.sel');
		if(seletDev.length == 0){
			$('div.dev_list li:eq(1)').addClass('sel');
			try{PBrecFileTableInit();}catch(e){}
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
		var date = $("div.calendar span.nowDate").html();dev
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
	var typeHint = [];
		typeHint[1] = '定时';
		typeHint[2] = '运动';
		typeHint[4] = '警告';
		typeHint[8] = '手动';
		typeHint[15] = '全部';
	function ocxsearchVideo(){
		try{
			recTotal = 0;
			$('tbody.search_result tr').filter(function(){
				return !$(this).find(':checkbox').is(':checked');
			}).remove();//远程备份中正在下载的文件不被删除.
			oPlayBack.GroupStop();
			oPlaybackLocl.GroupStop();
			dragStopMove();
		}catch(e){
			//alert('try:'+e);
		}
		var devData = $('#dev_'+$('div.dev_list li.sel span.channel').data('data').dev_id).data('data');
		var type = $('#type span').attr('type') || 0;
			type = type == 0 ? 15 : 1 << type;
		var date = $("div.calendar span.nowDate").html();
		var startTime =gettime($('div.timeInput:eq(0) input')) || '00:00:00';
		var endTime =gettime($('div.timeInput:eq(1) input')) || '23:59:59';
		setDevData2ocx();
		/*show(chl+'+'+type+'+'+startTime+'+'+endTime);
		alert(oPlayBack.startSearchRecFile(chl,type,startTime,endTime));*/
		if(bool){
			oPlayBack.style.height='0px';
			oPlaybackLocl.style.height='100%';
			var chl ='';
			for (var i=1;i<=devData.channel_count;i++){
				chl+=i+';';
			};
			if(oPlaybackLocl.searchDateByDeviceName(devData.name)){
				alert('设备'+devData.name+'在本地没有录像!  '+oPlaybacKLocl.searchDateByDeviceName(devData.name));
				return false;
			}
			oPlaybackLocl.searchVideoFile(devData.name,date,startTime,endTime,chl);
		}else{
			var chl = 0;
			try{
				oPlaybackLocl.style.height='0px';
				oPlayBack.style.height='100%';
			}catch(e){}

			for (var i=0;i<devData.channel_count;i++){
				chl += 1 << i;
			};
			if(oPlayBack.startSearchRecFile(chl,type,date+' '+startTime,date+' '+endTime)!=0){
				alert('控件检索设备'+devData.name+'的'+typeHint[type]+'录像失败');
			}		
		}
	}
	function showRecProgress(now){
		//show(now+'//'+recTotal);
		var con = '正在检索:',
			p =now/recTotal*100;
		if(recTotal == now){
			con = '检索完成:';
		}
		$('#fileRec').find('span').width(p-2)
			             .end().find('h5').html(now+'/'+recTotal)
			             .end().find('h4').html(con);
	}
	function RecfinishCallback(data){
		recTotal = data.total ? data.total : 0;	
		
	}