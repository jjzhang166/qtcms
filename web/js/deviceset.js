
	$(function(){
		var oTreeWarp = $('div.dev_list').slice(2);
		$('ul.filetree').treeview().find('span.cam').click(function(){
			$(this).toggleClass('cam_1')
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

		$('#modifyDevice input:radio').each(function(index){ //添加设备弹出框下添加设备方式切换
			$(this).click(function(){
				var oSwitch = $('#modifyDevice tr').slice(2,6);
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
			$(this).mouseup(function(event){ 
				event.stopPropagation();	
				var obj = $(event.target);
				/*if(obj[0].nodeName == 'SPAN' && (obj.hasClass('area')|| obj.hasClass('group') )){ 
					alert(obj.data('data'));
				}*/
				if(event.which == 1){
					if( obj[0].nodeName == 'SPAN'){
						if(obj.hasClass('cam')){
							$('div.dev_list span').not('span.cam').removeClass('sel');
						}else{ 
							$('div.dev_list span').removeClass('sel');
						}
						obj.toggleClass('sel');
					}
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

		set_contentMax();
		
		$('#set_content div.left li').each(function(index){
			$(this).click(function(){
				$('#set_content div.right div').filter(function(){
					return $(this).parent().is('#set_content div.right');
				}).hide().eq(index).show();
				oTreeWarp.show();
				if(index == 0){
					searchFlush();
					areaList2Ui();
				}else if(index == 3){ 
					userList2Ui();
				}
			})
		})
		//搜索结果 设备列表tr委托部分事件;
		$('#SerachDevList tbody').on('click','tr',function(){
			$(this).find('input:checkbox').click();
		})
		$('#SerachDevList tbody').on('click','input:checkbox',function(event){
			event.stopPropagation();
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
	})///
	
	function set_contentMax(){
		var W = $(window).width();
		var H = $(window).height();
		$('#set_content .right').width(W - 250);
		$('#set_content .right').height(H - 106);
		$('#set_content .left').height(H - 106);
		$('#foot').css({
			top:H - 28
		})
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
	function areaList2Ui(){
		$('div.dev_list span.area').not('#area_0').parent('li').remove();
		//$('ul.filetree').treeview();
		var areaListArrar=[];
		var pidList=[];
		var areaList = oCommonLibrary.GetAreaList();
		for(n in areaList){
			var id = areaList[n];
			var name = oCommonLibrary.GetAreaName(areaList[n]);
			var pid = oCommonLibrary.GetAreaPid(areaList[n]);
			areaListArrar.push([id,name,pid]);
			pidList.push(pid);

		}
		var arr =del(pidList.sort(sortNumber));;  //  返回pid升序的PID数组
		for(j in arr){
			for(k in areaListArrar){		
				if(areaListArrar[k]['2'] == arr[j]){		
					var add = $('<li><span class="area" id="area_'+areaListArrar[k]['0']+'">'+areaListArrar[k]['1']+'</span><ul></ul></li>').appendTo($('div.dev_list:eq(0) #area_'+arr[j]).next('ul'));
					add.find('span.area').data('data',areaListArrar[k]);
					$('ul.filetree:eq(0)').treeview({add:add});
				}
			}
		}
		for(i in areaListArrar){
			deviceList2Ui(areaListArrar[i][0])		;
		}
	}
	function groupList2Ui(){ 
		var groupList = oCommonLibrary.GetGroupList();
		//var names = ['group1','group2','group3'];
		for( i in groupList){
			var id = groupList[i];
			var name =oCommonLibrary.GetGroupName(id);
			var add = $('<li><span class="group" id="group_'+id+'">'+name+'</span><ul></ul></li>').appendTo($('#group_0').next('ul'));
			add.find('span.group').data('data',[id,name]);
			$('ul.filetree:eq(1)').treeview({add:add});
		}
	}

	function deviceList2Ui(areaid){
		var devList = oCommonLibrary.GetDeviceList(areaid);
		for (i in devList){ 
			var name=devList[i];
			//var name = oCommonLibrary.GetDeviceName(id);
			var add = $('<li><span class="device" >'+name+'</span><ul></ul></li>').appendTo($('#area_'+areaid).next('ul'));
			//add.find('span.group').data('data',[id,name]);
			$('ul.filetree:eq(1)').treeview({add:add});

		}
	}

	function ShowUserOperateMenu(obj){  //显示弹出层 调整定位。
		var node = $('#'+obj+'');
		node.find('input:text,:password').val('');
		if(obj == 'menu3' || obj == 'confirm'){
			var str = $('table.UserMan input:hidden').val().split(',');
			if(str == ''){ 
				alert('please select user!!!');
				return false;
			}
			if(obj == 'menu3'){
				node.find('input:first').val(str[0]);
			}
		}
		objShowCenter(node);
	}

	function objShowCenter(obj){ //调整弹出框定位 居中
		$('#iframe').show();
		obj.css({
			top:($(window).height() - obj.height())/2,
			left:($(window).width() - obj.width())/2
		}).show();
	}
	function pwdTest(obj){
		var str = obj.val();
		if(str == '' || !/[\d\w_]{4,}$/.test(str)){
			alert('请用下划线数字字母组合且长度大于6位');
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
			alert('2次密码不一样');
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
		$('#iframe').hide();
		$('#menusList div.menu').hide();
		$('#menusList div.menu input.data').remove();
		//$('#menusList div.menu input:hidden').remove();
		$('#menusList div.menu input:text').val('');
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
		}else if(obj.hasClass('cam')){ 
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
function addDev(){
	var add = $('<li><span class="area">Area</span><ul><li><span class="device" >device</span><ul><li><span class="cam">Channel01</span></li><li><span class="cam">Channel02</span></li><li><span class="cam">Channel03</span></li><li><span class="cam">Channel04</span></li><li><span class="cam">Channel05</span></li><li><span class="cam">Channel06</span></li><li><span class="cam">Channel07</span></li><li><span class="cam">Channel08</span></li></ul></li></ul></li>').appendTo('div.dev_list:eq(0) ul.filetree');
	$('ul.filetree').treeview({add:add});

}
//遮罩层和弹出框方法.
var trance = {'area':'区域','device':'设备','cam':'通道','group':'分组','Add':'增加','Remove':'删除','Modify':'修改'};
function showObjActBox(action,objclass){
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
	var objdata = $('span.sel').data('data');
	if(action == 'Remove'){ 
		var obox = $('#confirm');
		obox.find('div.confirm').attr('id','Remove'+firstUp(objclass)+'_ok');
		$('<input class="data" type="hidden" value="'+objdata[0]+'" />').appendTo('#confirm');
	}else{
		var obox = $('#'+objclass);
		initActionBox(action,pObj,obox,objclass);
	}
	obox.find('p span').html(trance[action]+trance[objclass]);
	objShowCenter(obox);
}
function initActionBox(action,pObj,obox,objclass){
	var data = pObj.data('data');
	var pObjType = firstUp(pObj.attr('class').split(' ')[0]);
	obox.find('div.confirm').attr('id',action) //action+pObjType+'_ok'
	if(action == 'Add'){
		if(objclass == 'device'){ 
			obox.find('div.confirm').attr('id',firstUp(action+objclass)+'_ok')
		}
		obox.find('input.parent'+pObjType).val(data[1]);
	}else if(action == 'Modify'){
		if(pObjType == 'Area' || pObjType == 'Group'){ 
			obox.find('#'+pObjType+'_Name_ID').val(data[1]);
			obox.find('input.parent'+pObjType).val($('#'+pObjType.toLowerCase()+'_'+data[2]).html());
			$('<input type="hidden" id="Pid_ID" value="'+data[2]+'"/><input type="hidden" id="'+pObjType+'_id_ID" value="'+data[0]+'"/>').appendTo(obox);
		}
			/*obox.find('#Area_Name_ID').val(data[1]);
			obox.find('input.parentArea').val($('#area_'+data[2]).html());
			$('<input type="hidden" id="Pid_ID" value="'+data[2]+'"/><input type="hidden" id="Area_id_ID" value="'+data[0]+'"/>').appendTo(obox);*/
	}
}
// 辅助方法.
function del(str) {   //数组去除重复
	var a = {}, c = [], l = str.length; 
	for (var i = 0; i < l; i++) { 
	var b = str[i]; 
	var d = (typeof b) + b; 
	if (a[d] === undefined) { 
	c.push(b); 
	a[d] = 1; 
	} 
	} 
	return c; 
}

function sortNumber(a,b){ //数组生序排列
	return a-b;
}

function firstUp(str){  //字符串首字母大写
	var a = str.split('');
	a[0] = a[0].toUpperCase();
	return a.join('');
}