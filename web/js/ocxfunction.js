var oSelected = [],
	oSearchOcx,
	oCommonLibrary;
	var oActiveEvents = ['Add','Delete','ModifyUserLevel','ModifyUserPasswd','AddArea','ModifyArea','RemoveArea','AddGroup','RemoveGroup','ModifyGroup','ModifyChannel','AddDevice','ModifyDevice','RemoveDevice','AddDeviceDouble','AddChannelDoubleInGroup'];  //事件名称集合
	$(function(){ 
		$('#area_0').data('data',{'area_id':'0','area_name':'区域_root','pid':'0','pareaname':'root'})
		$('#group_0').data('data',{'group_id':'0','group_name':'分组_root'});// 主区数据句填充.
		oCommonLibrary = document.getElementById('commonLibrary')
		oSearchOcx = document.getElementById('devSearch');
		//搜索设备;
		oSearchOcx.AddEventProc('SearchDeviceSuccess','callback(oJson);');
		oSearchOcx.Start();
		//分组列表;
		groupList2Ui();
		//区域列表;
		areaList2Ui();
		//debug();
		for (i in oActiveEvents){
			AddActivityEvent(oActiveEvents[i]+'Success',oActiveEvents[i]+'Success(data)');
			AddActivityEvent(oActiveEvents[i]+'Fail','Fail(data)');
		}
	})	

	function AddSuccess(ev){
		var name = document.getElementById('add_username').value;
		var usrLev= $('#menu2 div.select span').html();
		var No = $('#UserMan table.UserMan tbody tr').length - 1;
		$('<tr><td>'+No+'</td><td>'+name+'</td><td>'+usrLev+'</td></tr>').appendTo('#UserMan table.UserMan');		
	}
	function AddFaild(ev){
		Confirm('添加失败!');
	}
	function DeleteSuccess(){
		oSelected = [];
		$('#UserMan table.UserMan tbody tr input:hidden').val('');
		$('#UserMan table.UserMan tbody tr').filter(function(){ 
			return $(this).attr('class') == 'selected';
		}).remove();
	}
	function DeleteFaild(ev){ 
		Confirm('删除失败');
	}
	function Fail(data){
		var str = '<p>';
		if(data.name){
			str +=data.name+':';
		}else if(data.channelname){ 
			str +=data.name+':';
		}
		str+=data.fail+'</p>';
		Confirm('<p>'+data.name+':'+data.fail+'</p>');
	}
	function ModifyUserLevelSuccess(ev){
		$('#UserMan table.UserMan tbody tr').filter(function(){ 
			return $(this).attr('class') == 'selected';
		}).eq(0).find('td:last').html($('#menu3 div.select span').html());
	}

	//搜索设备控件方法.
	function searchFlush(){
		$('#SerachDevList tbody tr').remove();	 
		oSearchOcx.Flush();
	}
	//设备搜索回调函数
	function callback(oJson){
		var bUsed = true;
		$('#SerachDevList tbody tr').each(function(){ 
			if(parseInt($(this).find('td:eq(1)').html()) == oJson.SearchDeviceId_ID || $(this).find('td:eq(2)').html() == oJson.SearchIP_ID){
				bUsed = false;
			}
		})
		if(bUsed){
			$('<tr id="esee_'+oJson.SearchDeviceId_ID+'"><td><input type="checkbox"/>'+oJson.SearchVendor_ID+'</td><td>'+oJson.SearchDeviceId_ID+'</td><td>'+oJson.SearchIP_ID+'</td><td>'+oJson.SearchChannelCount_ID+'</td></tr>').appendTo($('#SerachDevList tbody')).data('data',oJson);
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
		var add = $('<li><span class="group" id="group_'+id+'">'+name+'</span><ul></ul></li>').appendTo($('#group_0').next('ul'));
			add.find('span.group').data('data',{'group_id':id,'group_name':name});
			$('ul.filetree:eq(1)').treeview({add:add});
		closeMenu();
	}
	function RemoveGroupSuccess(data){ 
		var id=$('#group_id_ID').val();
		$('#group_'+id).next('ul').find('span.channel').each(function(){ 
			var devid = $(this).data('data')['dev_id'];
			var add = $(this).parent('li').appendTo($('div.dev_list:eq(0) #dev_'+devid).next('ul'));
			$('ul.filetree').treeview({add:add});
		})
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
	function AddDeviceSuccess(data){
		var dataIndex={'area_id':'','address':'','port':'','http':'','eseeid':'','username':'','password':'','device_name':'','channel_count':'','connect_method':'','vendor':'','dev_id':data.deviceid,'parea_name':$('#parea_name_ID').val()}
		for(i in dataIndex){ 
			if(dataIndex[i] == ''){
				dataIndex[i] = $('#device #'+i+'_ID').val();
			}
		}
		adddev(dataIndex);
		
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
	function AddDeviceDoubleSuccess(data){
		/*data = {};
		data.name = $('#SerachDevList input:checked').parent('td').parent('tr').attr('id').split('_')[1];
		data.deviceid = '50';*/
		Confirm('<p>'+data.name+'AddSuccess</p>');
		var area = $('div.dev_list:eq(0) span.sel:eq(0)').hasClass('area') ? $('div.dev_list:eq(0) span.sel:eq(0)') : $('div.dev_list:eq(0) span.area:eq(0)');
		var devData = $('#esee_'+data.name).data('data');
		var devData2={'area_id':area.data('data')['area_id'],'address':devData['SearchIP_ID'],'port':devData['SearchHttpport_ID'],'http':devData['SearchHttpport_ID'],'eseeid':data.name,'username':'admin','password':'','device_name':data.name,'channel_count':devData['SearchChannelCount_ID'],'connect_method':'0','vendor':devData['SearchVendor_ID'],'dev_id':data.deviceid,'parea_name':area.data('data')['area_name']};
		adddev(devData2);

	}
	function debug (){ 
		$('#area_0').data('data',{'area_id':'0','area_name':'区域_root','pid':'0','pareaname':'区域_root'});// 主区数据句填充.
		$('#area_1').data('data',{'area_id':'1','area_name':'test1','pid':'0','pareaname':'区域_root'});// 
		$('#group_0').data('data',['0',$('#group_0').html()]);// 主区数据句填充.
		$('#dev_1').data('data',{'devname':'test4','address':'123','eseeid':'10001685','username':'admin','devid':'1'});// 主区数据句填充.
		$('.debug').show();
	}
	function Confirm(str){
		$('#confirm').find('h4').html($('#confirm').find('h4').html()+str).show();
		$('div.close').html('确定')
		objShowCenter($('#confirm'));
	}
	function adddev(data){ 
		var add = $('<li><span class="device" id="dev_'+data.dev_id+'" >'+data.device_name+'</span><ul></ul></li>').appendTo($('#area_'+data.area_id).next('ul'));
		add.find('span.device').data('data',data);
		var chlList = oCommonLibrary.GetChannelList(data.dev_id);
		for(i in chlList){
			var chlNum = '';//oCommonLibrary.GetChannelNumber(chlList[i]);
			var num =parseInt(i)+1;
			var name = num < 10 ? 'channel'+'0'+num : 'channel'+num;
			var chldata={'channel_id':chlList[i],'dev_id':data.dev_id,'channel_number':chlNum,'channel_name':name,'stream_id':'0'};
			var addchl = $('<li><span class="channel" id="channel_'+chlList[i]+'">'+chldata.channel_name+'</span></li>').appendTo($('#dev_'+data.dev_id).next('ul'));
			addchl.find('span.channel').data('data',chldata);
			$('ul.filetree:eq(0)').treeview({add:addchl});
		}
		$('ul.filetree:eq(0)').treeview({add:add});
	}
	function AddChannelDoubleInGroupSuccess(data){
		Confirm('<p>'+data.channelname+'AddSuccess</p>');
		var group = $('div.dev_list:eq(1) span.sel:eq(0)').hasClass('group') ? $('div.dev_list:eq(1) span.sel:eq(0)') : $('div.dev_list:eq(1) span.group:eq(0)');
		var add = $('div.dev_list:eq(0) #channel_'+data.chlid).parent('li').appendTo(group.next('ul'));
		$('ul.filetree').treeview({add:add});
		$('ul.filetree').treeview();

	}