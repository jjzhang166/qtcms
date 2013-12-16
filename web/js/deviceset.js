var oActiveEvents = ['Add','Delete','ModifyUserLevel','ModifyUserPasswd','AddArea','ModifyArea','RemoveArea','AddGroup','RemoveGroup','ModifyGroup','ModifyChannel','AddDevice','ModifyDevice','RemoveDevice','AddDeviceDouble','AddChannelDoubleInGroup'];  //事件名称集合
var oSearchOcx;
	$(function(){
		oSearchOcx = document.getElementById('devSearch');
		var oTreeWarp = $('div.dev_list').slice(2);
		$('ul.filetree').treeview().find('span.channel').click(function(){
			$(this).toggleClass('channel_1')
		});
		oTreeWarp.hide();
		$('#iframe').hide();

		document.oncontextmenu = function(e){  //文档默认右键事件冒泡取消
			var e = e || window.event;
			if(e.target.tagName != 'BODY'){
				return false;
			}
		}
		$('div.menu .close').click(function(){  //弹出操作框下部分元素添加关闭窗口事件
			closeMenu();
		})

		$('#device input:radio').each(function(index){ //添加设备弹出框下添加设备方式切换
			$(this).click(function(){
				$('#device input:radio').attr('id','');
				$(this).attr('id','connect_method_ID');
				var oSwitch = $('#device tr').slice(2,6);
				oSwitch.removeClass('tr1').find('input').prop('disabled',false);
				if(index == 0){
					oSwitch.slice(3,4).addClass('tr1').find('input').prop('disabled',true);
				}else{
					oSwitch.slice(0,3).addClass('tr1').find('input').prop('disabled',true);
				}
			})
		})
		
		$('ul.filetree span.areaName').click(function(){
			$('ul.filetree span.areaName').css('background','0');
			$(this).css('background','#ccc');
		})
		$('div.dev_list').each(function(index){
			var This = $(this);
			This.mouseup(function(event){ 
				event.stopPropagation();	
				var obj = $(event.target);
				/*if(obj[0].nodeName == 'SPAN'){
					show(obj.data('data'));
				}*/
				if(event.which == 1){
					if( obj[0].nodeName == 'SPAN'){
						if(obj.hasClass('channel')){
							This.find('span').not('span.channel').removeClass('sel');
							if(!$('div.dev_list:eq(1) span.sel.group')[0]){
								$('div.dev_list:eq(1) span.group:eq(0)').addClass('sel');
							}
						}else{ 
							This.find('span').removeClass('sel');
						}
						obj.toggleClass('sel');
					}
					SetChannelIntoGroupData();
				}
				if(event.which == 3){ 
					if(obj[0].nodeName == 'SPAN'){ 
						$('div.dev_list span').removeClass('sel');
						obj.addClass('sel');
					}else{ 
						if(!$(this).find('span.sel')[0]){
							$(this).find('span:eq(0)').addClass('sel');
						}
					}
					//分组设置下的右键菜单调整
					$('#menu0 li').show()
					$('#menu0 li').eq(index).hide();
					if(index == 1){
						$('#menu0 li:eq(2)').hide();
					}
					showContextMenu(event.clientY,event.clientX,obj);
				}else{ 
					$('#menu0').hide();
				}
			})
		})	
			
		$('.hover').each(function(){  // 按钮元素添加鼠标事件对应样式
			var action = $(this).attr('class').split(' ')[0];
			addMouseStyle($(this),action);
		})
		
		$('#set_content div.left li').each(function(index){
			$(this).click(function(){
				$('#set_content div.right div').filter(function(){
					return $(this).parent().is('#set_content div.right');
				}).hide().eq(index).show();
				oTreeWarp.show();
				if(index == 0){
					searchFlush();
					areaList2Ui();
				}else if(index == 1){
					var areaList = oCommonLibrary.GetAreaList();
					var areaList0 = 0;
				
					$("#device_list li").remove();
					var name0 = oCommonLibrary.GetAreaName(0);
					var deviceList0 = oCommonLibrary.GetDeviceList(0);
					$('#device_list').treeview()	
							var add_li = $('<li><span class="area">'+name0+'<ul id="area0"></ul></li>');
							add_li.appendTo('#device_list');
							$("#device_list").treeview({
								add: add_li
							});	
						for(n in areaList){
							var id = areaList[n];
							var name = oCommonLibrary.GetAreaName(areaList[n]);
							var pid = oCommonLibrary.GetAreaPid(areaList[n]);
							
							$('#device_list').treeview()	
							
							var add_li = $('<li><span class="area">'+name+'<ul id="area'+id+'"></ul></li>');
							add_li.appendTo('#device_list');
							$("#device_list").treeview({
								add: add_li
							});	
							var deviceList = oCommonLibrary.GetDeviceList(id)
								for(k in deviceList)
								{
									var dev_id = deviceList[k];
									var deviceInfo = oCommonLibrary.GetDeviceInfo(dev_id);			
									$('#area'+id).treeview()	
									var add_li_0 = $('<li class="device1" value="'+dev_id+'"><span id="device'+dev_id+'" class="device11">'+deviceInfo.name+'</li>');
									add_li_0.appendTo('#area'+id);
									$('#area'+id).treeview({
										add: add_li_0
									});
								}
							}
							for(j in deviceList0)//手动添加area0的设备
							{
								var dev_id0 = deviceList0[j];
								var deviceInfo0 = oCommonLibrary.GetDeviceInfo(dev_id0);	
								$('#area0').treeview()	
								var add_li_1 = $('<li class="device1" value="'+dev_id0+'"><span id="device'+dev_id0+'" class="device11">'+deviceInfo0.name+'</li>');
								add_li_1.appendTo('#area0');
								$('#area0').treeview({
									add: add_li_1
								});
							}
						
						$('.device1').each(function(device_index){
							$(this).click(function(){
								var deviceInfo_0 = oCommonLibrary.GetDeviceInfo($(this)[0].value);
								var _url = 'http://'+deviceInfo_0.address+':'+deviceInfo_0.port;
								var _usr = deviceInfo_0.username;
								var _pwd = deviceInfo_0.password;
								var _chn = oCommonLibrary.GetChannelCount($(this)[0].value);
								for(j = 0;j<=10;j++){	
									$('.ipc_list').eq(j).hide();	
									$('.dvr_list').eq(j).hide();	
									$('.dvr_list0').eq(j).hide();		
									$('.ipc_list0').eq(j).hide();	
									$('.ope_list li').removeClass('act');					
									if(deviceInfo_0.vendor == 'JUAN IPC'){//如果选中设备为ipc
										$('.ipc_list0')[0].style.display='block';									
										$('.ipc_list0 li').eq(0).addClass('act');
										$('.ipc_list').eq(0).show();
										ipc(_url,_usr,_pwd);
										devinfo_load_content(true);										
									}
								    if(deviceInfo_0.vendor == 'JUAN DVR')//如果选中设备为dvr
									{
										$('.dvr_list0')[0].style.display='block';
										$('.dvr_list0 li').eq(0).addClass('act');
										$('.dvr_list').eq(0).show();
										dvr(_url,_usr,_pwd,_chn);
										dvr_devinfo_load_content();	
										}
								}		
							})
					    })
				}
				else if(index == 3){ 
					userList2Ui();
				}
			})
			//搜索设备;
			//oSearchOcx.AddEventProc('SearchDeviceSuccess','callback(oJson);');
			//searchFlush();
			/*for (i in oActiveEvents){
				AddActivityEvent(oActiveEvents[i]+'Success',oActiveEvents[i]+'Success(data)');
				AddActivityEvent(oActiveEvents[i]+'Fail','Fail(data)');
			}*/
		})
		//搜索结果 设备列表tr委托部分事件;
		$('#SerachDevList tbody').on('click','tr',function(){
			$(this).find('input:checkbox').click();
		})
		$('#SerachDevList tbody').on('click','input:checkbox',function(event){
			event.stopPropagation();
			var devList = $('#SerachDevList tbody input:checked');
			var oArea=$('div.dev_list:eq(0)')
			if(!oArea.find('span.sel')[0]){
				oArea.find('span').removeClass('sel');
				oArea.find('span.area:first').addClass('sel');
			}
			var areaID = oArea.find('span.sel').data('data')['area_id'];
			var str = '<devListInfo cut = "'+devList.length+'" area_id="'+areaID+'">';
			devList.each(function(){ 
				var data = $(this).parent('td').parent('tr').data('data');
				var dataStr = '<dev username="admin" password="" ';
				for(i in data){ 
					dataStr+=i+'="'+data[i]+'" ';
				}
				dataStr+=' />';
				str+=dataStr;
			});
			str+=' </devListInfo>';
			$('#adddevicedouble_ID').val('').val(str);
			//alert($('#adddevicedouble_ID').val());
			var sDevId = parseInt($(this).parent('td').next('td').html());
			if( sDevId <= 0 || !sDevId){ 
				return false;
			}
		})
		//用户table下 tr委托部分事件
		$('table.UserMan').on('click','tr',function(){  //添加用户 tr选中状态添加  数据整合到 hidden的input
			//整理选中的用户ID数组
			var userName = $(this).find('td').eq(1).html();
			if($(this).attr('class')){
				for(i in oSelected){
					if(oSelected[i] == userName){ 
						oSelected.splice(i,1);
					}
				}
			}else{
				oSelected.push(userName);	
			}
			$(this).toggleClass('selected');  // tr toggle样式
			$(this).parent('tbody').find('input:hidden').val(oSelected.join());
		})
		set_contentMax();
	})///
	$(window).resize(function(){ 
		set_contentMax();
	})
	//分组区域添加到分组, 数据组织
	function SetChannelIntoGroupData(){
		var oChlList = $('div.dev_list:eq(0) span.sel.channel');
		var oArea = $('div.dev_list:eq(1) span.sel.group');
		var str = '<chlintogroup cut="'+oChlList.length+'" group_id="'+oArea.data('data')['group_id']+'">';
		oChlList.each(function(){ 
			var sChl = '<chlinfo ';
			var data = $(this).data('data')
			for(i in data){ 
				sChl+=i+'_ID="'+data[i]+'" ';
			}
			sChl+='/>';
			str+=sChl
		})
		str+='</chlintogroup>';
		$('#addchannelingroupdouble_ID').val('').val(str);
		//alert($('#addchannelingroupdouble_ID').val());
	}
	function set_contentMax(){
		var W = $(window).width();
		var H = $(window).height();
		$('#set_content .right').width(W - 250);
		$('#set_content .right').height(H - 106);
		$('#set_content .left').height(H - 106);
	}
	//用户设置方法 User Manage
	function userList2Ui(){
		$('#UserMan table.UserMan tbody input:hidden').val('');
		$('#UserMan table.UserMan tbody tr').not(':first').remove();
		var userList = oCommonLibrary.GetUserList();
		if(userList.length){  //避免数组为空的时候. 自己写的JS数组扩展方法引起 BUG;
			for(i in userList){
				var userlv = oCommonLibrary.GetUserLevel(userList[i]);
				var userCom;
				switch(userlv){
					case 0 : userCom = '超级管理员';	break;
					case 1 : userCom = '管理员';	break;	
					case 2 : userCom = '普通用户'; break;
					case 3 : userCom = '游客'; break;
					default: userCom = '游客'; break;
				}
				$('<tr><td>'+i+'</td><td>'+userList[i]+'</td><td>'+userCom+'</td></tr>').appendTo('#UserMan table.UserMan');
			}
		}		
	}
	
	function ShowUserOperateMenu(obj){  //显示弹出层 调整定位。
		var node = $('#'+obj+'');
		node.find('input:text,:password').val('');
		if(obj == 'menu3' || obj == 'confirm'){
			var str = $('table.UserMan input:hidden').val().split(',');
			if(str == ''){ 
				Confirm('please select user!!!');
				return false;
			}
			if(obj == 'menu3'){
				node.find('input:first').val(str[0]);
			}
		}
		objShowCenter(node);
	}

	function objShowCenter(obj){ //调整弹出框定位 居中
		$('#iframe').hide().show();
		obj.css({
			top:($(window).height() - obj.height())/2,
			left:($(window).width() - obj.width())/2
		}).show();
	}
	function pwdTest(obj){
		var str = obj.val();
		if(str == '' || !/[\d\w_]{4,}$/.test(str)){
			Confirm('请用下划线数字字母组合且长度大于6位');
		}else{
			obj.attr('enable','1');
			valueIsSame($('#add_again_passwd'),$('#add_passwd'));
			valueIsSame($('#modify_again_passwd'),$('#modify_newpasswd'));
		}
	}
	function valueIsSame(obj,obj2){ 
		if(obj.val() && obj2.val() && obj.val() != obj2.val()){ 
			obj.attr('enable','0');
			obj2.attr('enable','0');
			Confirm('2次密码不一样');
			return;
		}else{ 
			obj.attr('enable','1')
			obj2.attr('enable','1')
		}
	}
	function showMenu(id,str){
		if(str){ 
			$('#'+id).find('p span').html(str);
		}
		$('#iframe').show();
		objShowCenter($('#'+id));
	}
	function initModifyMenu(){ 
		var oDevData = $('#SerachDevList tbody tr').filter(function(){ 
			return $(this).find('input:checkbox').is(':checked')
		}).eq(0).data('data');
	}
	function closeMenu(){ 
		$('#iframe,div.confirm,#menusList div.menu').hide();
		$('#menusList div.menu input.data').remove();
		$('#menusList div.menu input:text').val('');
		$('#confirm h4').html('');
		$('div.close').html('取消');
	}
	function removeArry(obj,number){ 
		for(i in obj){ 
			if(obj[i] == number){ 
				obj.splice(i,1);
			}
		}
		return obj;
	}
//设备搜索. 设备设置. 分组设置. 区域设置.
function showContextMenu(y,x,obj){
	var menu = $('#menu0')
	menu.find('li').off();
	if(obj[0].nodeName == 'SPAN'){
		if(obj[0].id == 'area_0' || obj[0].id == 'group_0' ){
			$('#menu0 li:gt(2)').hide();
		}else if(obj.hasClass('group')){
			menu.find('li:lt(2)').hide();
		}else if(obj.hasClass('device')){ 
			menu.find('li:lt(3)').hide();
		}else if(obj.hasClass('channel')){ 
			menu.find('li').not(':eq(3)').hide();
			menu.find('li:eq(3)').show();
		}	
		menu.find('li:eq(3)').one('click',function(){ 
			showObjActBox('Modify',obj.attr('class').split(' ')[0]);
		})
		menu.find('li:last').one('click',function(){ 
			showObjActBox('Remove',obj.attr('class').split(' ')[0]);
		})
	}else{ 
		$('#menu0 li:gt(2)').hide();
	}
	menu.css({
		top:y,
		left:x,					
	}).show(10,function(){ 
		$(document).mouseup(function(event){
			event.stopPropagation();
			menu.hide();
			$(document).off();
		})
	});
}

//遮罩层和弹出框方法.
var trance = {'area':'区域','device':'设备','channel':'通道','group':'分组','Add':'增加','Remove':'删除','Modify':'修改'};
function showObjActBox(action,objclass){  //右键弹出菜单UI调整
	var pObjClass = objclass == 'group'?'group':'area';
	var pObj = $('span.sel');
	if(action == 'Add'){ // 调整添加的父级对象
		if(!$('span.sel')[0] || !$('span.sel').hasClass(pObjClass)){
			pObj = $('span.'+pObjClass+':eq(0)');
		}
	}else{ 
		if(!$('span.sel')[0]){
			$('span.'+objclass+':eq(0)').addClass('sel');
		}
	}
	if(action == 'Remove'){ 
		var obox = $('#confirm');
	}else{
		var obox = $('#'+objclass);
	}
	obox.find('p span').html(trance[action]+trance[objclass]);
	initActionBox(action,pObj,obox,objclass);
}
function initActionBox(action,pObj,obox,objclass){  //右键菜单数据填充.
	var data = pObj.data('data');
	if(!data){ 
		Confirm('请选择一台设备!');
		return false;
	}
	var pObjType = firstUp(pObj.attr('class').split(' ')[0]);
	$('#'+action+pObjType+'_ok').show();
	/*if(pObj.parent('li').parent('ul').prev('span').hasClass('group')){ 
		$('#channel input:text').attr('id','r_chl_group_name_ID');
	}else{ 
		$('#channel input:text').attr('id','channel_name_ID');
	}*/
	for(i in data){
		if(i.match(objclass+'_name') && action == 'Remove'){
			$('#confirm h4').attr('id',i+'_ID').html('删除'+data[i]);
		}
		var str = ''
		if(data[i] != ''){ 
			str = data[i];
		}
		if($('#'+i+'_ID')[0]){ 
			$('#'+i+'_ID').val(str);
		}else{ 
			$('<input type="hidden" class="data" value="'+str+'" id="'+i+'_ID"/>').appendTo(obox);
		}
	}
	if(action == 'Add'){
		$('#'+action+firstUp(objclass)+'_ok').show();
		$('#'+objclass+'_name_ID').val('');
		$('#area #'+objclass+'_id_ID').remove();
		$('#group #'+objclass+'_id_ID').remove();
		$('#pid_ID').val(data['area_id']);
		obox.find('input.parent'+pObjType).val(data['area_name']);
	}
	objShowCenter(obox);
}
//devinfo
