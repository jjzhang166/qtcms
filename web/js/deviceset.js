
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
				if( obj[0].nodeName == 'SPAN'){
					if(obj.hasClass('cam')){
						$('div.dev_list span').not('span.cam').removeClass('sel');
					}else{ 
						$('div.dev_list span').removeClass('sel');
					}
					obj.addClass('sel');		
				}
				if(event.which == 3){ 
					//分组设置下的右键菜单调整
					$('#menu0 li').show()
					$('#menu0 li').eq(index).hide();
					if(index == 1){ 
						$('#menu0 li').not(':eq(0)').hide();
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
		if(obj.hasClass('area') || obj.hasClass('group')){
			menu.find('li:eq(2)').hide();
		}else if(obj.hasClass('device')){ 
			menu.find('li:eq(1)').hide();
		}else if(obj.hasClass('cam')){ 
			menu.find('li').not(':eq(3)').hide();
			menu.find('li:eq(3)').show();
		}
		menu.find('li:eq(3)').one('click',function(){ 
			contextmenue2Modify(obj);
		})
		menu.find('li:last').one('click',function(){ 
			contextmenue2Del(obj);
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
//treeview 下右键菜单中的修改
var trance = {'area':'区域','device':'设备','cam':'通道','group':'分组'};
function contextmenue2Modify(obj){ 
	var key = obj.attr('class').split(' ')[0];
	initModifyDevMenu(key);
	showMenu(key,'修改'+trance[key]);
}
function contextmenue2Del(obj){ 
	var key = obj.attr('class').split(' ')[0];
	showMenu('confirm','删除'+trance[key]);
}
function initModifyDevMenu(key){ 
	if($('div.dev_list span.sel').length !=0){ 
		var node = $('div.dev_list span.sel');
		if(!node.hasClass(key)){ 
			$('div.dev_list span').removeClass('sel');
			var node = $('div.dev_list span.'+key+':first');
			node.addClass('sel');
		}
	}else{ 
		$('div.dev_list span').removeClass('sel');
		var node = $('div.dev_list span.'+key+':first');
		node.addClass('sel');
	}
	if(key == 'cam'){ 
		$('#'+key).find('input').val(node.html());
	}else if(key == 'device'){ 
		alert('设备数据填充中');
	}else{ 
		$('#'+key).find('input[readonly]').val(node.html());
	}
}
