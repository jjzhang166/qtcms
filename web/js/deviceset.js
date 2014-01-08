var oActiveEvents = ['Add','Delete','ModifyUserLevel','ModifyUserPasswd','AddArea','ModifyArea','RemoveArea','AddGroup','RemoveGroup','ModifyGroup','ModifyChannel','AddDevice','ModifyDevice','RemoveDevice','AddDeviceDouble','AddChannelDoubleInGroup','SettingStorageParm','SettingCommonParm','SettingRecordTime'];  //事件名称集合
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
		$('div.dev_list:lt(2)').each(function(index){
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
				var warp = $('#set_content div.right div.right_content').hide().eq(index).show();
				oTreeWarp.show();
				set_contentMax();
				if(index == 0){
					$('div.dev_list span.device').parent('li').remove();
					searchFlush();
					//区域列表;
					areaList2Ui('0');
				}else if(index == 1){
					$('#device_list').find('li').remove();	
					var areaList = oCommonLibrary.GetAreaList();
					var areaList0 = 0;				
					//手动添加区域0
					var deviceList0 = oCommonLibrary.GetDeviceList(0);	
							var add_li = $('<li><span class="area">区域_root</span><ul id="area0"></ul></li>').appendTo('#device_list');
							$('ul.filetree:eq(2)').treeview({add:add_li});
							
					for(n in areaList){
						var id = areaList[n];
						var name = oCommonLibrary.GetAreaName(areaList[n]);
						var pid = oCommonLibrary.GetAreaPid(areaList[n]);
						
						$('#device_list').treeview()	
						
						var add_li = $('<li><span class="area">'+name+'</span><ul id="area'+id+'"></ul></li>').appendTo('#device_list');
						$("#device_list").treeview({add: add_li});	
					//搜索添加区域
					var deviceList = oCommonLibrary.GetDeviceList(id)
						for(k in deviceList)
						{
							var dev_id = deviceList[k];
							var deviceInfo = oCommonLibrary.GetDeviceInfo(dev_id);			
							var add_li = $('<li class="device1" value="'+dev_id+'"><span class="device" id="device'+dev_id+'">'+deviceInfo.name+'</span></li>').appendTo('#area'+id);
							$('ul.filetree:eq(2)').treeview({add:add_li});
						}
					}
					//手动添加area0的设备
						for(j in deviceList0)
						{
							var dev_id0 = deviceList0[j];
							var deviceInfo0 = oCommonLibrary.GetDeviceInfo(dev_id0);	
							
							var add_li_1 = $('<li class="device1" value="'+dev_id0+'"><span class="device" id="device'+dev_id0+'">'+deviceInfo0.name+'</span></li>').appendTo('#area0');
							$('ul.filetree:eq(2)').treeview({add:add_li_1});
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
				}else if(index == 2){
					window['Fill'+warp.find('div.switch:visible').attr('id')+'Data']();
				}else if(index == 3){ 
					userList2Ui();
				}
			})
			//搜索设备;
			oSearchOcx.AddEventProc('SearchDeviceSuccess','callback(oJson);');
			searchFlush();
			for (i in oActiveEvents){
				AddActivityEvent(oActiveEvents[i]+'Success',oActiveEvents[i]+'Success(data)');
				AddActivityEvent(oActiveEvents[i]+'Fail','Fail(data)');
			}
			// 设置相关
			$('ul.dvr_list0').each(function(){//dvr
				$(this).toSwitch_0();
			});

			$('ul.ipc_list0').each(function(){//ipc
				$(this).toSwitch_1();
			});

			$('ul.ope_list').each(function(){    //设置项目UI相应逻辑.
				$(this).toSwitch();
			});
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
	function FillCommonParmData(){ 
		var item = ['Language','AutoPollingTime','SplitScreenMode','AutoLogin','AutoSyncTime','AutoConnect','AutoFullscreen','BootFromStart'];
		for(i in item){
			var str = oCommonLibrary['get'+item[i]]();
			var obj = $('#'+item[i]+'_ID')
			if(obj[0].nodeName != 'INPUT'){
				obj.dataIntoHtml(str).dataIntoVal(str)
			}else{
				obj.dataIntoSelected(str);
			}
		}

		$('#CommonParm input:checkbox').each(function(){ 
			$(this).toCheck();
		})
		$('#viewMod a').click(function(){
			$('#SplitScreenMode_ID').val('div'+$(this).html());
		})
	}
	function FillRecordTimeData(){
		areaList2Ui('3');
		$('ul.week a').each(function(index){ 
			$(this).click(function(){ 
				$('#week').html($(this).html());
			})
		})
		$('div.dev_list span.channel').click(function(){ 
			var chlData = $(this).data('data');
			alert(oCommonLibrary.GetRecordTimeBydevId(chlData.channel_id));
		})
		$('#RecordTime div.timeInput input').blur(function(){ 
				var str = '<recordtime num="4">';
				$('#RecordTime div.timeInput input').each(function(index){ 
					if(index%6 == 0){
						var timeid = $(this).prev('input:hidden').val();
						var start = $(this).parent('div.timeInput').gettime();
						var end = $(this).parent('div.timeInput').next('div.timeInput').gettime();
						var week = $('#week').val();
						str+='<num'+(parseInt(index/6)+1)+' recordtime_ID="'+timeid+'" starttime_ID="'+start+'" endtime_ID="'+end+'" enable_ID="'+week+'" />'
					}
				})
				str +='</recordtime>';
				$('#recordtimedouble_ID').val(str);
				//alert($('#recordtimedouble_ID').val());
		})
	}
	function FillStorageParmData(){
		var diskcheckbox = $('#StorageParm table table input:checkbox')
		var disks = oCommonLibrary.getEnableDisks().split(':');
		for(i in disks){ 
			diskcheckbox.each(function(){ 
				if(disks[i] == $(this).val()){ 
					$(this).prop('disabled',false);
				};
			})
		}
		var item = ['FilePackageSize','LoopRecording','DiskSpaceReservedSize']
		var useDisks = oCommonLibrary.getUseDisks().split(':');
		var clickedAbles = diskcheckbox.filter(function(){ 
			return !$(this).prop('disabled');
		})
		for( n in useDisks){ 
			clickedAbles.each(function(){ 
				$(this).dataIntoSelected(useDisks[n]);
			})
		}

		for( k in item){ 
			var str = oCommonLibrary['get'+item[k]]();
			var obj = $('#'+item[k]+'_ID');
			if(obj.is(':checkbox')){
				obj.dataIntoSelected(str);
			}else{ 
				obj.dataIntoVal(str).dataIntoHtml(str);
			}
		}

		clickedAbles.each(function(){
			$(this).click(function(){
				var checked = []; 
				clickedAbles.each(function(){ 
					$(this).is(':checked') == true && checked.push($(this).val());
					$('#UseDisks_ID').val(checked.join(':'));
				})
			})
		})
		$('#LoopRecording_ID').toCheck();
	}
	
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
		var W = $(window).width(),
			H = $(window).height(),
			now =0,
			oWarp ={};
		$('#set_content div.right').css({ 
			width:W - 250,
			height:H - 106
		}).find('div.right_content').each(function(index){
			if($(this).is(':visible')){
				now = index;
				oWarp = $(this);
			}
		})

		$('#set_content div.left').height(H - 106);
		if(now == 0){ 
			var main = $('#SerachDevList');
			main.css({ 
				height:H-272
			})
			/*if(main.width()>760){ 
				main.width(780);
			}*/
			$('#Allocation').css({ 
				top:main.height()+2
			})
			oWarp.find('div.action').css('left',main.width()-30);
			$('#left_list').css('left',main.width()+116);
		}
		$('#foot').css('top',H-28)
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
function selectAll(){
	$('#SerachDevList tbody input:checkbox').click();/*.on('click','tr',function(){
		$(this).find('input:checkbox').click();
	})*/
}
function disksSelectAll(){
	$('#StorageParm input:checkbox:lt(22)').click();
}
function test(){ 
	alert(123);
}
