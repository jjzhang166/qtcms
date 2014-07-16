var auth,nowDev,
	ERROR = '',
	base64 = new Base64(),

	AJAX = null, //当前发送ajax请求的对象;
	type='get',
	dataType='json',
	jsonp='',
	async=true;

$.ajaxSetup({
	processData: false, 
	cache: false,
	timeout:5000
})

function data2UI(objData,oWarp){

	if(nowDev._USR && nowDev._USR == 'no auth'){
		showAJAXHint('Unauthorized').show();
	}
	var defaultWarp = $('#set_content div.ipc_list:visible')
	var warp = (oWarp && oWarp[0]) ? oWarp : defaultWarp;

	for(var i in objData){

			var dataKey = '[data-UI~="'+i+'"]',
				warpKey = '[data-WARP~="'+i+'"]'
			if(typeof objData[i] == 'object' && ignoreKey(i)){
				/*/^\d+$/.test(objData[i][0])*/
				if(objData[i]['opt']){
					warp.find('ul'+warpKey).html('');
					for(var k in objData[i]['opt']){
						warp.find('ul'+warpKey).append('<li><input type="text" value="'+objData[i]['opt'][k]+'"></li>');
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
							default: 
								oTag.val(lang[objData[i]] ? lang[objData[i]] : objData[i]).attr('data',objData[i]);
							break;
						}
					}
				}/*else{
					console.log('ignoreKey-----------------'+i);
				}*/	
			}
	}
}

function ignoreKey(key){
	var ignoreKey = ['type','mode','ver','seq','object','__proto__','upnp','wireless'];
	for(var i=0;i<ignoreKey.length;i++){
		if(key == ignoreKey[i]){
			return false;
		}
	}
	return true;
}

function _AJAXget(url,data,beforeSend,success,complete){   //  get方法

	async && emptyDevSetMenu(); //异步请求时清空表单.

	type = 'GET'

	__AJAXconstruct(url,data,beforeSend,success,function(){
		dataType='json';
		jsonp='';
		
		typeof(complete) == 'function' && complete();
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

	var str = type == 'GET' ? 'loading' : 'saveing' ;

	var targetMenu = $('#set_content div.ipc_list:visible');

		oHint = showAJAXHint(str).css('top',targetMenu.height() + 46);

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
			str =  type == 'GET' ? 'loading_success' : 'save_success';
			//console.log('---------------ajaxSuccess------------------');

			showAJAXHint(str);
			var Data = jsonp ? xml2json.parser(data.xml,'', false) : data;

			console.log(Data);
			typeof(success) == 'function' && success(Data);
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){

			str = 'loading_fail';

			if(lang[textStatus]){
				str = textStatus
			}

			if(errorThrown=='Unauthorized'){
				str='Unauthorized';				
				nowDev._VER = 'no auth';
			}

			str != 'abort' && showAJAXHint(str);

/*
			if(str != 'abort'){
				console.log('++++++++++++++++XMLHttpRequest+++++++++++++++');
				console.log(XMLHttpRequest.status);

				console.log('++++++++++++++textStatus+++++++++++++++++');

				console.log(textStatus);

				console.log('++++++++++++++++errorThrown+++++++++++++++');

				console.log(errorThrown);

				console.log('+++++++++++++++++++++++++++++++');
			}*/
			
			//alert("error:" + textStatus);
		},
		complete: function(XMLHttpRequest, textStatus){
			console.log('-------------complete-----------'+str);
			if(str != 'abort'){
				if(str == 'loading_success' || str == 'save_success'){
					oHint.fadeOut(2000);
					checkAJAX();
				}else{
					showAJAXHint(str);
				}

				typeof(complete) == 'function' && complete();
			}
		}
	});
}

//IPC
function PCTime2dev(){
	$('#curent_time').val($('#PC_time').val());
}

function checkAJAX(){

	var a = '';

	if(nowDev._VER && nowDev._VER == 'no auth'){
		$('#set_content div.ipc_list:eq(0) input:text').val('');
		a = 'Unauthorized';

		console.log('=======nowDev._VER=========='+nowDev._VER);

	}else if(nowDev._VER < nowDev._Upgrade){
		a = 'low_ver';
	}

	a && showAJAXHint(a)

	return a;
}

//setting
function submitThisMenu(str){
	nowDev[str+'Put']();
}

function getPutDataJSON(){
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
}

function disable(warp,str){
	var oInput = $('#set_content div.ipc_list:visible [data-WARP="'+warp+'"]').find('input:text,input:password');

	str ? oInput.prop('disabled',str) : oInput.removeProp('disabled');
	 $('#set_content div.ipc_list:visible div.select').find('input').prop('disabled',true)
	 												  .end().next('ul.option').find('input').prop('disabled',true);
	
}

//检车通道名是否为中文.

function hasChinese(str){
	if(/[\u4e00-\u9fa5]+/g.test(str)){
		showAJAXHint('channelName_NOT_chinese').val('');
	}else{
		showAJAXHint().hide();
	}
}

function showAJAXHint(str){
	var t = lang[str] || str;
	return $('#ajaxHint').html(t).stop(true,true).show();
}
//检查IP格式
function chkInput(type,obj,str){
	var hint = '';
		val = obj.value;
	switch(type){  //IPC编码设备码率
		case 0:
		var max =  $(obj).attr('max'),
			min = $(obj).attr('min');
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
			if(parseInt(val) > 65535 || parseInt(val) < 0 || val == ''){
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

function reInitNowDev(){
	if(nowDev && key == 1){
		var odev = $('#dev_'+nowDev._ID).addClass('sel')
		odev.parent('li').siblings('li').find('span').removeClass('sel');
		var oDevData = odev.data('data');
		nowDev = new IPC(oDevData.username,oDevData.password,oDevData.address,oDevData.port,oDevData.dev_id,oDevData.vendor);
	}
}


function portAsync(){
	$('#http_ID_Ex').val($('#port_ID_Ex').val());
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