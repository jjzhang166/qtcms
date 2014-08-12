var auth,//用户信息
    nowDev,//当前设备信息
	ERROR = '',
	base64 = new Base64(),

	AJAX = null, //当前发送ajax请求的对象;
	type='get',
	dataType='json',
	jsonp='',
	async=true;

$.ajaxSetup({ //不变的全局变量
	processData: false, 
	cache: false,
	timeout:5000
})

function data2UI(objData,oWarp){ //

	if(nowDev._USR && nowDev._USR == 'no auth'){
		showAJAXHint('Unauthorized').show();
	}
  var defaultWarp = $('#set_content div.switch:visible');
  // var defaultWarp = $('#set_content div.dvr_list:visible');
	var warp = (oWarp && oWarp[0]) ? oWarp : defaultWarp;

	for(var i in objData){ //objData 指的是xmlstr

			var dataKey = '[data-UI~="'+i+'"]',
				warpKey = '[data-WARP~="'+i+'"]'
			//console.log(dataKey+"------dataKey");
			//console.log(warpKey+"------warpKey");
			if(typeof objData[i] == 'object' && ignoreKey(i)){  //判断从objData数组中取出来的数据是否为 object && 可忽略的选项变量）
				/*/^\d+$/.test(objData[i][0])*/
				if(objData[i]['opt']){  //取出来的数据如果是opt类型
					warp.find('ul'+warpKey).html('');
					for(var k in objData[i]['opt']){
						warp.find('ul'+warpKey).append('<li><input type="text" value="'+objData[i]['opt'][k]+'" disabled></li>');
					}
				}
		        
				//console.log('obj-----------------'+i);

				warp = warp.find(warpKey)[0] ? warp.find(warpKey) : defaultWarp.find(warpKey);

				warp = warp[0] ? warp : defaultWarp;

				data2UI(objData[i],warp);
				
			}else{
				if(ignoreKey(i)){

					var oTag = warp.find(dataKey)[0] ? warp.find(dataKey) : defaultWarp.find(dataKey);

				if(oTag[0]){
					/*console.log('__data.key: __ |'+i+'| __data.key.value: __ |'+objData[i]+'|  ____warp.attr("data-UI")~= '+ warp.attr('data-UI')
							+'____oTag.attr("data-UI") = '+ oTag.attr('data-UI'));
					console.log(warp);
					console.log(oTag);*/
						/*warp.css('border','1px solid red');
						console.log(warp.attr('type'));*/
                       //if(oTag.attr('type')){
						   //console.log(oTag.attr('type'));
						switch(oTag.attr('type')){
							case 'radio' :
								/*console.log('+++++++++++++++radio++++++'+i+'+++++++++++'+objData[i]);
								console.log(warp);
								console.log(oTag);*/

								warp.find(':radio[data-UI="'+i+'"][value="'+objData[i]+'"]').prop('checked',true);
							break;
							case 'checkbox' :
								oTag.prop('checked',objData[i]);
							break;
							case 'select':
								oTag.val(oTag.parent('div.select').next('ul.option').find('input:eq('+objData[i]+')').val());
							break;
							default: 
								oTag.val(lang[objData[i]] ? lang[objData[i]] : objData[i]).attr('data',objData[i]);
							break;
						}
					}
				}
				//}else{
				//	oTag.val(oTag.parent().next().find('li').eq(objData[i]).val());
				//}
			}
	}
}


function ignoreKey(key){//不常用的选项变量
//'mode'由于DVR的编码设置和报警设置中有用到，但是IPC不知道在哪里有屏蔽掉，所以下次排查要注意。
	var ignoreKey = ['type','ver','seq','object','__proto__','upnp','wireless'];
	for(var i=0;i<ignoreKey.length;i++){
		if(key == ignoreKey[i]){
			return false;
		}
	}
	return true;
}

function _AJAXget(url,data,beforeSend,success,complete){   //  get方法

	if(typeof(beforeSend) != 'boolean' && async) emptyDevSetMenu(); //异步请求时清空表单.

	type = 'GET';

	__AJAXconstruct(url,data,beforeSend,success,function(str){
		dataType='json';
		jsonp='';
		
		typeof(complete) == 'function' && complete(str);
	});								
}


function _AJAXput(url,data,beforeSend,success,complete){ // put方法
	var b = true;

	$('#set_content div.ipc_list:visible input[data-UI]').each(function(){
		if($(this).attr('b')){
			b = false;
		}
	})

	if(!b){
		showAJAXHint('check_to_put');
		return;
	}

	console.log('-------------------_AJAXget  init------------------------');

	type ='PUT'

	__AJAXconstruct(url,data,beforeSend,success,complete);
}

function __AJAXconstruct(url,data,beforeSend,success,complete){  //AJAX 初始化方法.

	var str = type == 'GET' ? 'loading' : 'saveing',

		warp = $('#set_content div.switch:visible').find('input').attr("disabled",true).end(); //确保DVR和IPC界面的提示信息在适当的位置

		if(typeof(beforeSend) != 'boolean'){
			var oHint = showAJAXHint(str).css('top',warp.height() + 46);	
		}		

	//console.log('----------usr:--'+nowDev._USR+'-----password:--'+nowDev._PWD+'----');

	if(checkAJAX())
		return;

	//AJAX && AJAX.abort();

	AJAX=$.ajax({
		type:type,
		url: url,
		data: data,
		dataType: dataType,
		jsonp: jsonp,
		async:async,
		beforeSend: function(re){
			//console.log('-----------ajaxSend:'+auth+'--------------');
			//console.log('-----------url:'+url+'---------------');
			re.setRequestHeader('Authorization',auth);
			
			//console.log(re);
			typeof(beforeSend) == 'function' && beforeSend();
		},
		success: function(data,textStatus,jqXHR){
			/*console.log('----------usr:--'+nowDev._USR+'-----password:--'+nowDev._PWD+'----')
			console.log('++++++++++++++++jqXHR+++++++++++++++');
			console.log(jqXHR.status);

			console.log('++++++++++++++textStatus+++++++++++++++++');

			console.log(textStatus);

			console.log('+++++++++++++++++++++++++++++++');*/

			var Data = jsonp ? xml2json.parser(data.xml,'', false) : data;

			console.log(Data);

			//console.time('数据填充时间');
			typeof(success) == 'function' && success(Data);
			//console.timeEnd('数据填充时间');

			warp.find('input').filter(function(){ 
				var a = $(this).parent()[0].nodeName
				return  ((a=='TD' || a == 'DIV') && $(this).parent().attr('class') != 'select');
			}).attr("disabled",false);

			str =  type == 'GET' ? 'loading_success' : 'save_success';
			//console.log('---------------ajaxSuccess------------------');
			if(typeof(beforeSend) != 'boolean'){
				showAJAXHint(str);
			}
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){

			str = 'loading_fail';

			if(lang[textStatus]){
				str = textStatus
			}

			if(errorThrown=='Unauthorized'){
				str='Unauthorized';				
			}

			if(str != 'abort' && typeof(beforeSend) != 'boolean'){
				nowDev._VER = str;
				showAJAXHint(str);
			}

			/*if(str != 'abort'){
				console.log('++++++++++++++++XMLHttpRequest+++++++++++++++');
				console.log(XMLHttpRequest.status);

				console.log('++++++++++++++textStatus+++++++++++++++++');

				console.log(textStatus);

				console.log('++++++++++++++++errorThrown+++++++++++++++');

				console.log(errorThrown);

				console.log('+++++++++++++++++++++++++++++++');
			}*/
		},
		complete: function(XMLHttpRequest, textStatus){
			
			warp.find('input').filter(function(){ 
				var a = $(this).parent()[0].nodeName
				return  ((a=='TD' || a == 'DIV') && $(this).parent().attr('class') != 'select');
			}).attr("disabled",false);
			
			console.log('-------------complete-----------'+str);

			typeof(complete) == 'function' && complete(str);
		
			if(str == 'abort' || typeof(beforeSend) == 'boolean' )return;

			if(str == 'loading_success' || str == 'save_success'){
				oHint.fadeOut(2000);
				checkAJAX();
			}else{
				showAJAXHint(str);
			}	
		}
	});
}

//IPC
//设备时间跟PC同步设置 按钮函数
function PCTime2dev(){
 
	showAJAXHint('saveing');

	dataType='jsonp';
	jsonp='jsoncallback';

	$('#curent_time').val($('#PC_time').val());//当前时间设为PC的时间
	var nowTime = new Date();
    
	//xml数据
	var xmlstr = '';
	xmlstr += '<juan ver="1.0" seq="0">';
	xmlstr += '<setup type="write" user="admin" password="">';
	xmlstr += '<time value="' + nowTime.getTime()/1000 + '" >';
	xmlstr += '</time>';
	xmlstr += '</setup>';
	xmlstr += '</juan>';
   //用ajaxget方法来保存设备上的时间，所以自己写提示信息
	_AJAXget(nowDev.getRequestURL()+'/cgi-bin/gw2.cgi?f=j','xml='+xmlstr,false,'',function(str){

		dataType='json';
		jsonp='';

		if(str == 'loading_success' || str == 'save_success'){
			showAJAXHint('save_success').fadeOut(2000);
			checkAJAX();
		}else{
			showAJAXHint(str);
		}
	});

}
//检测设备软件版本和用户登录情况
function checkAJAX(){
	var warp = $('#set_content div.ipc_list:visible')
	//console.log(nowDev);
	var a = '';
	if(nowDev._VER < nowDev._Upgrade){ //当前设备软件版本低于CMS 支持的最低版本IPC
		a = warp.attr('action') != "ipcBasicInfo" ? 'low_ver' : '';//提示信息
	}else{
		if(!/^(\w+_?)+$/.test(nowDev._VER)){
			a = '';
		}else{
			a = nowDev._VER;//提示信息
			if(nowDev._VER == 'Unauthorized'){ //默认用户:admin,默认空密码.登陆失败!
				$('#set_content div.ipc_list:eq(0) input:text').val(''); //表单置为空
				//console.log('=======nowDev._VER=========='+nowDev._VER);
			}
		}
	}

	a && showAJAXHint(a) 

	return a;
}

//提交保存按钮触发的函数，根据相应的ID+put调用相应的函数进行数据保存
function submitThisMenu(str){
	$('#ajaxHint').html('').stop(true,true).hide();//提示隐藏
	AJAX && AJAX.abort();//多次请求时（即连续点击保存按钮时）,只响应一次
	nowDev[str+'Put']();
}

/*function getPutDataJSON(){
	var warp = $('#set_content div.ipc_list:visible'),
		str = '{',
		node = {};
	warp.find('input[data-UI]').each(function(){
		if($(this).is('checked')){
			node = warp.find('[data-UI="'+$(this).attr('data-UI')+'"]').is('checked');
		}else{
			node = $(this);
		}
		var val = node.val();

		if(val == 'false'){
			val = false; 
		}else if(val == 'true'){
			val = true; 
		}else if(/^\d+$/.test(val)){
			val = parseInt(val);
		}else{
			val = '"'+val+'"';
		}
		str += '"'+node.attr('data-UI')+'":'+val+',';
	})

	str = str.slice(0,-1) + '}';

	return str;
}*/
//input表单的可写/不可写
function disable(warp,str){
	var oInput = $('#set_content div.ipc_list:visible [data-WARP="'+warp+'"]').find('input:text,input:password');

	 str ? oInput.prop('disabled',str) : oInput.removeProp('disabled');
	 $('#set_content div.ipc_list:visible div.select').find('input').prop('disabled',true)
	 												  .end().next('ul.option').find('input').prop('disabled',true);
	
}
//dvr表单的可写、不可写
function dvr_disable(data_ui){
	
	var warp = $('#set_content div.dvr_list:visible').find('input[data-UI="'+data_ui+'"]');
	var oInput = warp.parents('table').find('input:text,input:password,input[type="select"]');
	
	if(warp.prop('checked') == true){
			oInput.prop('disabled',false);
		}else{
			oInput.prop('disabled',true);
		}
	
	}
/*
   --检车通道名是否为中文--
匹配中文字符的正则表达式： [\u4e00-\u9fa5] 
匹配双字节字符(包括汉字在内)：[^\x00-\xff]
/[\u4e00-\u9fa5]+/g  多个汉字
*/ 
function hasChinese(str){
	if(/[\u4e00-\u9fa5]+/g.test(str)){
		showAJAXHint('channelName_NOT_chinese').val('');
	}else{
		showAJAXHint().hide();
	}
}
//输出显示提示信息
function showAJAXHint(str){
	var t = lang[str] || str;
	return $('#ajaxHint').html(t).stop(true,true).show();
}
//检查IP格式

function chkInput(type,obj,str){//设备类型  当前input对象 “文本框前的名称”

	var hint = '';
		val = obj.value;
	switch(type){  
		case 0: //IPC编码设备码率
		var max =  $(obj).attr('max'),
			min = $(obj).attr('min');
			// /^\d+$/ 是正则表达式  ^和$用来匹配位置: ^表示行首,$表示行尾  \d表示数字,即0-9  +表示重复1次以上搜索综合起来,/^\d+$/ 这个正则表达式就是匹配一整行1个以上的数字
			if(!/^\d+$/.test(val) || (parseInt(val) > max || parseInt(val) < min)){ 
				hint = _T(str)+'('+min+'~'+max+')';
			}
		break;
		case 1:
			var re=/^(([1-9]|([1-9]\d)|(1\d\d)|(2([0-4]\d|5[0-5])))\.)(([1-9]|([1-9]\d)|(1\d\d)|(2([0-4]\d|5[0-5])))\.){2}([1-9]|([1-9]\d)|(1\d\d)|(2([0-4]\d|5[0-5])))$/;
			if(!re.test(val)){
				hint = _T(str);
			}
		break;
		case 2:
			var re=/^(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])(\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])){3}$/;
			if(!re.test(val)){
				hint = _T(str);
			}
		break;
		case 3:
			if(parseInt(val) > 65535 || parseInt(val) < 0 || !/^\d{1,5}$/.test(val)){
				hint = _T(str);
			};
		break;
	}

	if(hint){
		$(obj).css('border','1px dashed red').attr('b',1);
		showAJAXHint(_T('correct')+hint);
	}else{
		$(obj).css('border','0').removeAttr('b');
		showAJAXHint().hide();
	}
}

function getEncode(data){
	//console.log(data);
	nowDev._ipcencodeInfo2UI(data);
}
//重新加载IPC/DVR设备信息
function reInitNowDev(){
	if(nowDev && key == 1){
		var odev = $('#dev_'+nowDev._ID).addClass('sel')
		odev.parent('li').siblings('li').find('span').removeClass('sel');
		var oDevData = odev.data('data');
		if(oDevData.vendor=='IPC'){
		   nowDev = new IPC(oDevData.username,oDevData.password,oDevData.address,oDevData.port,oDevData.dev_id,oDevData.vendor);
		}else{
		   nowDev = new DVR(oDevData.username,oDevData.password,oDevData.address,oDevData.port,oDevData.dev_id,oDevData.vendor);
			}
	}
}


function portAsync(){
	$('#http_ID_Ex').val($('#port_ID_Ex').val());
}
//DVR 点击通道数时触发的事件
//屏幕设置
function getTitle(num){
   nowDev._dvrScreenInfo2UI(num);
}
//编码设置
function getencode(num){

	nowDev._dvrencodeInfo2UI(num);
}
//云台设置
function getPTZ(num){
	
	nowDev._dvrPTZInfo2UI(num);
}
//视频检测设置
function getVideo(num){
	
	nowDev._dvrVideoCkeck2UI(num);
}
//警报设置
function getAlarm(num){
	
	nowDev._dvrAlarmInfo2UI(num);	
}
//录像设置
function getRecord(num,week){
	
	nowDev._dvrVideoInfo2UI(num,week);	
}

/*function AJAXabort(){
	for(i in AJAX){
		AJAX[i].abort();
	}
}*/
/*function json2str(obj){
  var S = [];
  for(var i in obj){
	  obj[i] = typeof obj[i] == 'string'?'"'+obj[i]+'"':(typeof obj[i] == 'object'?json2str(obj[i]):obj[i]);
	  S.push('"'+i+'":'+obj[i]); 
  }
  return '{'+S.join(',')+'}';
}*/
