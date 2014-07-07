var auth,nowDev,
	type='get',
	dataType='json',
	jsonp='',
	ERROR = '',
	base64 = new Base64(),
	AJAX = null; //当前发送ajax请求的对象;

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

	emptyDevSetMenu();

	type = 'GET'

	__AJAXconstruct(url,data,beforeSend,success,function(){
		dataType='json';
		jsonp='';
		
		typeof(complete) == 'function' && complete();
	});								
}


function _AJAXput(url,data,beforeSend,success,complete){ // put方法

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

	AJAX = $.ajax({
		type:type,
		url: url,
		data: data,
		dataType: dataType,
		jsonp: jsonp,
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

			showAJAXHint(str)
			var Data = jsonp ? xml2json.parser(data.xml,'', false) : data;

			//console.log(Data);
			typeof(success) == 'function' && success(Data);
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
			/*console.log('++++++++++++++++XMLHttpRequest+++++++++++++++');
			console.log(XMLHttpRequest.status);

			console.log('++++++++++++++textStatus+++++++++++++++++');

			console.log(textStatus);

			console.log('++++++++++++++++errorThrown+++++++++++++++');

			console.log(errorThrown);

			console.log('+++++++++++++++++++++++++++++++');*/

			str = 'loading_fail'

			if(errorThrown && lang[errorThrown]){
				str=errorThrown;				
			}

			if(str=='Unauthorized'){
				nowDev._VER = 'no auth';
			}
			
			showAJAXHint(str);
			//alert("error:" + textStatus);
		},
		complete: function(XMLHttpRequest, textStatus){
			console.log('-------------complete-----------'+str);

			if(str == 'loading_success' || str == 'save_success'){
				oHint.fadeOut(2000);
				checkAJAX();
			}else{
				showAJAXHint(str);
			}

			typeof(complete) == 'function' && complete();

			//console.log(/(\d+\\.)+/.test(nowDev._VER));
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
	return $('#ajaxHint').html(lang[str]).stop(true,true).show();
}
//检查IP格式
function chkIPformat(str){
	return;
	var re=/^(\d{1,3}\.){3}\d{1,3}$/;
	console.log(str+'//'+re.test(str));
	if(re.test(str)){
		showAJAXHint().hide();
	}else{
		showAJAXHint('Incorrect_IP_format');
	}
}

function getEncode(data){
	console.log(data);
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

function portIPToCMS(){
	console.log('--修改端口和地址重新同步设备状态----')
	areaList2Ui();
	console.log('--修改端口和地址重新加载设备列表----');
	reInitNowDev();
}

/*function json2str(obj){
  var S = [];
  for(var i in obj){
	  obj[i] = typeof obj[i] == 'string'?'"'+obj[i]+'"':(typeof obj[i] == 'object'?json2str(obj[i]):obj[i]);
	  S.push('"'+i+'":'+obj[i]); 
  }
  return '{'+S.join(',')+'}';
}*/