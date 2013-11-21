var oSelected = [],
	oSearchOcx,
	oCommonLibrary;	
	$(function(){ 
		oCommonLibrary = document.getElementById('commonLibrary')
		oSearchOcx = document.getElementById('devSearch');

		oSearchOcx.AddEventProc('SearchDeviceSuccess','callback(oJson);');
		oSearchOcx.Stop();
		oSearchOcx.Start();
	})

	var oActiveEvents = ['Add','Delete','ModifyUserLevel','ModifyUserPasswd'];  //事件名称集合	

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
		
	} 
	function aAddAreaFail(){ 

	}