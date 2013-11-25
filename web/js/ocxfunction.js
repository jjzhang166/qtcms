var oSelected = [],
	oSearchOcx,
	oCommonLibrary;	
	$(function(){ 
		oCommonLibrary = document.getElementById('commonLibrary')
		oSearchOcx = document.getElementById('devSearch');

		//搜索设备;
		$('#area_0').data('data',['0',$('#area_0').html(),'root']);// 主区数据句填充.
		$('#group_0').data('data',['0',$('#group_0').html()]);// 主区数据句填充.
		/*$('#area_1').data('data',['1',$('#area_1').html(),'0']);// 主区数据句填充.
		$('#group_1').data('data',['1',$('#group_1').html(),'0']);// 主区数据句填充.*/

		oSearchOcx.AddEventProc('SearchDeviceSuccess','callback(oJson);');
		oSearchOcx.Start();
		setTimeout(function(){oSearchOcx.Stop()},5000);
		//区域列表;
		areaList2Ui();
		groupList2Ui();
	})

	var oActiveEvents = ['Add','Delete','ModifyUserLevel','ModifyUserPasswd','AddArea','ModifyArea','RemoveArea'];  //事件名称集合	

	function AddSuccess(ev){
		var name = document.getElementById('add_username').value;
		var usrLev= $('#menu2 div.select span').html();
		var No = $('#UserMan table.UserMan tbody tr').length - 1;
		$('<tr><td>'+No+'</td><td>'+name+'</td><td>'+usrLev+'</td></tr>').appendTo('#UserMan table.UserMan');		
	}
	function AddFaild(ev){
		alert('添加失败!');
	}
	function DeleteSuccess(){
		oSelected = [];
		$('#UserMan table.UserMan tbody tr input:hidden').val('');
		$('#UserMan table.UserMan tbody tr').filter(function(){ 
			return $(this).attr('class') == 'selected';
		}).remove();
	}
	function DeleteFaild(ev){ 
		alert('删除失败');
	}
	function ModifyUserLevelSuccess(ev){
		$('#UserMan table.UserMan tbody tr').filter(function(){ 
			return $(this).attr('class') == 'selected';
		}).eq(0).find('td:last').html($('#menu3 div.select span').html());
	}
	function ModifyUserLevelFaild(){ 
		alert('修改失败');
	}
	function ModifyUserPasswdSuccess(){
		alert('密码修改');
	} 

	function ModifyUserPasswdFaild(){ 
		alert('密码修改失败');
	}

	for (i in oActiveEvents){
		AddActivityEvent(oActiveEvents[i]+'Success',oActiveEvents[i]+'Success(ev)');
		AddActivityEvent(oActiveEvents[i]+'Faild',oActiveEvents[i]+'Faild(ev)');
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
			if(parseInt($(this).find('td:eq(1)').html()) == oJson.SearchVendor_ID){
				bUsed = false;
			}
		})
		if(bUsed){
			$('<tr><td><input type="checkbox"/>'+oJson.SearchVendor_ID+'</td><td>'+	oJson.SearchDeviceId_ID+'</td><td>'+oJson.SearchIP_ID+'</td><td>'+oJson.SearchChannelCount_ID+'</td></tr>').appendTo($('#SerachDevList tbody')).data('data',oJson);
		}		
	}
	function AddAreaSuccess(){
		alert(1)
	} 
	function AddAreaFail(){ 
		alert(0)
	}
	function RemoveAreaSuccess(){
		var id = $('#confirm input:hidden').val();
		if(id != 0){
			$('div.dev_list').find('#area_'+id).parent('li').remove();
			$('ul.filetree').treeview();
		}
		closeMenu();
	}
	function RemoveAreaFail(){ 
		alert('删除区域失败');
	}
	function ModifyAreaSuccess(){
		var oArea = $('#area_'+$('#area #Area_id_ID').val());
		var sNewName = $('#area #Area_Name_ID').val()
		oArea.html(sNewName);
		oArea.data('data')[1]=sNewName;
		closeMenu();
	} 
	function ModifyAreaFail(){ 
		alert(0)
	}