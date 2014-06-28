var ERROR = '',auth,dataType='json',jsonp='',type='get',nowDev;

$.ajaxSetup({
	processData: false, 
	cache: false,
	timeout:5000,
	async:true,
})

function data2UI(objData,oWarp){

	if(nowDev._USR && nowDev._USR == 'no auth'){
		$('#ajaxHint').stop(true,true).html(lang.login_fail).show();
	}
	var defaultWarp = $('#set_content div.ipc_list:visible')
	var warp = (oWarp && oWarp[0]) ? oWarp : defaultWarp;

	for(var i in objData){

			var dataKey = '[data-UI~="'+i+'"]',
				warpKey = '[data-WARP~="'+i+'"]'
			if(typeof objData[i] == 'object'){
				/*/^\d+$/.test(objData[i][0])*/
				if(objData[i]['opt']){
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
								/*oTag.prop('checked',false).filter(function(){
									return $(this).val() == objData[i];
								}).prop('checked',true);*/
								//console.log('+++++++++++++++radio++++++'+i+'+++++++++++'+objData[i]);
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
	var ignoreKey = ['type','mode','ver','seq','object','__proto__'];
	for(var i=0;i<ignoreKey.length;i++){
		if(key == ignoreKey[i]){
			return false;
		}
	}
	return true;
}

function _AJAXget(url,data,beforeSend,success,complete){   //  get方法

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

	var str = type == 'GET' ? lang.loading : lang.saveing ;

	var targetMenu = $('#set_content div.ipc_list:visible');

		oHint = $('#ajaxHint').stop(true,true).css('top',targetMenu.height() + 46).html(str).show();

		if(checkAJAX())
			return false;

		/*if(/[\d+\\.]+/.test(nowDev._VER) && nowDev._VER < '1.3.0'){
			oHint.stop(true,true).html(lang.low_ver).show();
			return;
		}
*/
	/*if(nowDev._USR && nowDev._USR == 'no auth'){
		$('#ajaxHint').stop(true,true).html(lang.login_fail).show();
		return
	}

	if(nowDev._VER < '1.3.0'){
		$('#ajaxHint').stop(true,true).html(lang.low_ver).show();
	}*/

	$.ajax({
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
		success: function(data){
			str =  type == 'GET' ? lang.loading_success : lang.save_success;
			//console.log('---------------ajaxSuccess------------------');

			oHint.html(str);
			var Data = jsonp ? xml2json.parser(data.xml,'', false) : data;

			console.log(Data);
			typeof(success) == 'function' && success(Data);
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
			str = lang.loading_fail;

			if(textStatus)
				str=lang[textStatus];

			if(errorThrown){
				str=lang.login_fail;
			}

			oHint.html(str);
			//alert("error:" + textStatus);
		},
		complete: function(XMLHttpRequest, textStatus){

			if(str == lang.loading_success ||str == lang.save_success){
				oHint.fadeOut(2000);
			}else{
				oHint.stop(true,true).html(str);
			}

			checkAJAX();

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
		a = lang.login_fail;
	}else if(nowDev._VER < '1.3.0'){
		a = lang.low_ver;
	}

	a && $('#ajaxHint').stop(true,true).html(a).show();

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
	
}

/*function json2str(obj){
  var S = [];
  for(var i in obj){
	  obj[i] = typeof obj[i] == 'string'?'"'+obj[i]+'"':(typeof obj[i] == 'object'?json2str(obj[i]):obj[i]);
	  S.push('"'+i+'":'+obj[i]); 
  }
  return '{'+S.join(',')+'}';
}*/