var IPC = function(usr,pwd,ip,port,id,type){
	this._IP = ip;
	this._PORT = port;
	this._USR = usr;
	this._PWD = pwd;
	this._ID = id;
	this._TYPE = type;
	this._VER = 0;
	this._Upgrade = '1.3.0';  // CMS 支持的最低版本IPC

	_Request=[];

	auth = "Basic " + base64.encode(this._USR+':'+this._PWD);

	/*$(document).ajaxSend(function(re){
		console.log('--------------ajaxSend-------------------------');
		console.log(re);
		re.setRequestHeader('Authorization',auth);
	});*/


	this.getRequestURL = function(){
		//console.log('http://'+this._IP+':'+this._PORT);
		return 'http://'+this._IP+':'+this._PORT;
	}

	this.ipcBasicInfo2UI = function(){ //获取设备信息
        emptyDevSetMenu();
		reInitNowDev();
		var This = this;
		console.log('-------------------ipcBasicInfo2UI--------------------------');
		var xmlstr = '';
			xmlstr += '<juan ver="" seq="">';
			xmlstr += '<conf type="read" user="'+this._USR+'" password="'+this._PWD+'">';
			xmlstr += '<spec vin="" ain="" io_sensor="" io_alarm="" hdd="" sd_card="" />';
			xmlstr += '<info device_name="" device_model="" device_soc="" device_sn="" sensor_type="" hardware_version="" software_version="" build_date="" build_time="" />';
			xmlstr += '</conf>';
			xmlstr += '</juan>';

		dataType='jsonp';
		jsonp='jsoncallback';
		
		_AJAXget(this.getRequestURL()+'/cgi-bin/gw2.cgi?f=j','xml='+xmlstr,'',function(data){
			data2UI(data);
			This._VER = $('#set_content div.ipc_list:visible input[data-UI="software_version"]').val();
		});
	}

	this.ipcencodeInfo2UI = function(){ //获取编码信息
		console.log('-------------------ipcencodeInfo2UI--------------------------');
		$('#set_content div.ipc_list:visible input[data]:first').attr('data',101).val(lang.Main_stream);
		this._ipcencodeInfo2UI(101);
		//_AJAXget(this.getRequestURL() + '/netsdk/video/encode/channel/101/properties','','',data2UI);
	}

	this._ipcencodeInfo2UI = function(num){
		emptyDevSetMenu();
		var warp = $('#set_content div.ipc_list:visible');
			warp.find('input[data-ui="constantBitRate"],[data-ui="frameRate"]').removeAttr('max min');
		_AJAXget(this.getRequestURL() + '/netsdk/video/encode/channel/'+num+'/properties','','',function(data){
				data2UI(data);
				warp.find('input[data-UI="constantBitRate"]').attr({
					max:data.constantBitRateProperty.max,
					min:data.constantBitRateProperty.min
				}).end()
				  .find('input[data-UI="frameRate"]').attr({
				  	max:data.frameRateProperty.max,
					min:data.frameRateProperty.min
				  })
			});
	}

	this.ipcencodeInfoPut = function(){ //设置编码信息

		var warp = $('#set_content div.ipc_list:visible'),

			getVlue = this.getVlue,

			getData = this.getData;
	
		console.log('---------------put----------------------');
		var encodePutJSON = '{"channelName": "'+ getVlue('channelName') +'","resolution":"'+getVlue('resolution')+'","freeResolution":'+getVlue('freeResolution')+',"resolutionWidth":'+ getVlue('resolutionWidth') +',"resolutionHeight":'+ getVlue('resolutionHeight') +',"bitRateControlType":"'+ getData('bitRateControlType')+'","constantBitRate":'+  getVlue('constantBitRate') +',"frameRate":'+  getVlue('frameRate') +'}';

		/*var str = getPutDataJSON().replace('可变码率','VBR');
			str = str.replace('固定码率','CBR');*/
		//console.log(str);

		/*var oJSON = $.parseJSON(getPutDataJSON())
			oJSON.bitRateControlType = warp.find('input[data-UI="bitRateControlType"]').attr('data');
		console.log(json2str(oJSON));*/
		_AJAXput(this.getRequestURL() + '/netsdk/video/encode/channel/'+warp.find('input[data]:first').attr('data'),encodePutJSON);

	}

	this.ipcnetworkInfo2UI = function(){ //获取网络信息
		_Request=new Array(5);

		console.log('-------------------ipcnetworkInfo2UI--------------------------');

		emptyDevSetMenu();

		var This = this,

			warp = $('#set_content div.ipc_list:visible'),

			before = false,

			finish = function(a){
				This.checkMultRequests(a);
				warp.find('[data-WARP="lan"] [data-UI="addressingType"]:checked').val() == 'dynamic' ? disable('lan',true) : disable('lan');
				warp.find('[data-WARP="ddns"] [data-UI="enabled"]:checked').val() == 'true' ? disable('ddns') : disable('ddns',true);
				warp.find('[data-WARP="pppoe"] [data-UI="enabled"]:checked').val() == 'true' ? disable('pppoe') : disable('pppoe',true);
				warp.find('input[data-UI="mac"]').prop('disabled',true);
			}

			showAJAXHint('loading').css('top',warp.height() + 46);

		
		//async=false;

		_AJAXget(this.getRequestURL()+'/netsdk/System/deviceInfo/macAddress','',before,function(data){
			warp.find('input[data-UI="mac"]').val(data);
		},finish);

		_AJAXget(this.getRequestURL()+'/netsdk/Network/Interface/1','',before,data2UI,finish);

		_AJAXget(this.getRequestURL()+'/netsdk/Network/Esee','',before,function(data){
			warp.find('input[name="esee"][value="'+data.enabled+'"]').prop('checked',true);
		},finish);

		_AJAXget(this.getRequestURL()+'/netsdk/Network/Dns','',before,data2UI,finish);


		_AJAXget(this.getRequestURL()+'/netsdk/Network/Port/1','',before,function(data){
			warp.find('input[data-UI="value"]').val(data.value);
		},finish);

		//async=true;
	}

	this.ipcnetworkInfoPut = function(){ //设置网络信息
		var warp = $('#set_content div.ipc_list:visible');

		showAJAXHint(str).css('top',warp.height() + 46);

	    _Request=new Array(4);

		async=false;
        console.log('-------------------ipcnetworkInfoPut--------------------------');
		var This = this,

			str='saveing',

			end = this.checkMultRequests,

			getVlue = This.getVlue,

			getBoolean = This.getBoolean,

			interFaceJSON = '{"lan":{"addressingType":"'+getVlue('addressingType')+'", "staticIP": "'+getVlue('staticIP')+'","staticNetmask": "'+getVlue('staticNetmask')+'", "staticGateway": "'+getVlue('staticGateway')+'" },"pppoe": { "enabled": '+getBoolean('pppoe')+', "pppoeUserName": "'+getVlue('pppoeUserName')+'", "pppoePassword": "'+getVlue('pppoePassword')+'" }, "ddns": { "enabled": '+getBoolean('ddns')+', "ddnsProvider": "'+getVlue('ddnsProvider')+'", "ddnsUrl": "'+getVlue('ddnsUrl')+'", "ddnsUserName": "'+getVlue('ddnsUserName')+'", "ddnsPassword": "'+getVlue('ddnsPassword')+'" }}';

		_AJAXput(this.getRequestURL()+'/netsdk/Network/Esee','{"enabled":'+(getBoolean('esee'))+'}',false,'',end/*function(){
			console.log('Esee状态修改完成{"enabled":'+(warp.find('input[data-warp="esee"] input:checked').val()=='true'? true:false)+'}++++++++++2');
		}*/);

		_AJAXput(this.getRequestURL()+'/netsdk/Network/Dns','{"preferredDns": "'+getVlue('preferredDns')+'", "staticAlternateDns": "'+getVlue('staticAlternateDns')+'" }',false,'',end/*function(){
			console.log('Dns状态修改完成++++++++++3');
		}*/);

		_AJAXput(this.getRequestURL()+'/netsdk/Network/Port/1','{ "value": '+getVlue('value')+'}',false,'',end/*function(){
			This._PORT = warp.find('input[data-UI="value"]').val();
			console.log('端口修改成功+++++++++++++++++++++++++4'+This.getRequestURL());
		}*/);

		_AJAXput(this.getRequestURL()+'/netsdk/Network/Interface/1',interFaceJSON,async,'',function(a){
			end(a);
			qob.OnModifyDeviceEx();
			reInitNowDev();
		});

		async=true;
	}

	this.ipcuserManagementInfo2UI = function(){ //获取用户信息
         emptyDevSetMenu();
		dataType='';

		console.log('-------------------ipcuserManagementInfo2UI--------------------------');
		
		_AJAXget(this.getRequestURL()+'/user/user_list.xml','username='+this._USR+'&password='+this._PWD,'',function(data){
			$('user_list',data).children().not(':first').each(function(){
				$('<tr><td>'+$(this).attr('name')+'</td><td><input type="checkbox" '+__TRUE($(this).attr('admin'),'checked')+'/>管理<input type="checkbox" '+__TRUE($(this).attr('permit_setting'),'checked')+'/>设置<input type="checkbox" '+__TRUE($(this).attr('permit_playback'),'checked')+'/>回放</td><td><button '+__TRUE($(this).attr('edit_user'),'disabled')+' type="button" value="保存">保存</button><button '+__TRUE($(this).attr('del_user'),'disabled')+' type="button" value="删除">删除</button></td></tr>').appendTo($('#IPC_user_manage tbody'));
			})
		});
		function __TRUE(attr,a){
			if(a == 'checked'){
				a = attr == 'no' ? '' : a+'="'+a+'"';
			}else{
				a = attr == 'no' ? a+'="'+a+'"' : '';
			}
			console.log(attr+'---------------'+a);
			return a;
		}
	}

	this.ipczoneInfo2UI = function(){ //获取时区信息
	   emptyDevSetMenu();
		var warp = $('#set_content div.ipc_list:visible');

		showAJAXHint(str).css('top',warp.height() + 46);

		emptyDevSetMenu();
         
		_Request=new Array(3);

		var	str = 'loading',

			finish = this.checkMultRequests;

		$('#PC_time').val(renewtime());

		console.log('-------------------timeZone--------------------------');
		_AJAXget(this.getRequestURL()+'/Netsdk/system/time/timeZone','',false,function(data){
			$('#time_zone').val(data).attr('data',data);
		},finish);

		//console.log('-------------------ntp-------------'+this.getRequestURL()+'/netsdk/system/time/ntp-------------');
		_AJAXget(this.getRequestURL()+'/Netsdk/system/time/ntp','',false,data2UI,finish);

		//console.log('-------------------localTime-------------'+this.getRequestURL()+'/NetSDK/System/time/localTime-------------');
		_AJAXget(this.getRequestURL()+'/NetSDK/System/time/localTime','',false,function(data){
			var time = data.split('+')[0].replace('T','  ');
			$('#curent_time').val(time).attr('data',time);
			//console.log(data);
		},finish)

		//console.log('-------------------PCTime--------------------------');

		setInterval(function(){
			$('#PC_time').val(renewtime());
		},1000);

        function renewtime(){
			var myDate = new Date,

				yy=myDate.getFullYear(),

				mm=addZero(parseInt(myDate.getMonth())+1),

				dd=addZero(myDate.getDate()),

				hh=addZero(myDate.getHours()),

				mi=addZero(myDate.getMinutes()),

				ss=addZero(myDate.getSeconds());

			return yy + "-" + mm + "-" + dd + "  " + hh + ":" + mi + ":" + ss;
		}
		this.ipczoneInfoPut = function(){ //设置时区
			var warp = $('#set_content div.ipc_list:visible');

			showAJAXHint(str).css('top',warp.height() + 46);

			async=false;

			_Request=new Array(2);

			var	finish = this.checkMultRequests,
			 
			 	str="saveing";

			_AJAXput(this.getRequestURL()+'/netsdk/system/time/timeZone','"'+$('#time_zone').val()+'"',false,'',finish);

			_AJAXput(this.getRequestURL()+'/netsdk/system/time/ntp','{"ntpEnabled":'+(warp.find('input[data-UI="ntpEnabled"]:checked').val() == 'true' ? true : false)+',"ntpServerDomain":"'+warp.find('input[data-UI="ntpServerDomain"]').val()+'"}',false,'',finish)
		    
			async=true;
		}
	}

	this.initialSetup2UI = function(){
		$('#ajaxHint').html('').stop(true,true).hide();
	}
    
    this.rebootPut =function(){
		   
		var warp = $('#set_content div.ipc_list:visible'), 
			
			str = 'Restarting',

			This = this;

		showAJAXHint(str).css('top',warp.height() + 46);
					
	    var xmlstr = '';
		xmlstr += '<juan ver="1.0" seq="0">';
		xmlstr += '<setup type="write" user="' + This._USR + '" password="' + This._PWD + '">';
		xmlstr += '<system operation="reboot" />';
		xmlstr += '</setup>';
		xmlstr += '</juan>';
				
	    dataType='jsonp';  //数据类型
        jsonp='jsoncallback'; // 回调函数
			 
	    _AJAXget(this.getRequestURL()+'/cgi-bin/gw2.cgi?f=j','xml='+xmlstr,false,'',function(str){
		    if(str=='loading_success')
	           showAJAXHint('Restart_success').fadeOut(2000);
	        else
	          showAJAXHint(str);
	   	});
			
	}
    this.default_settingPut =function(){

    	var warp = $('#set_content div.ipc_list:visible'), 

		    str = 'factory_reseting',

		    This = this;

			//dev_id = $('div.dev_list:eq(2) span.device.sel').attr("id").slice(4);

    	showAJAXHint(str).css('top',warp.height() + 46);

		var xmlstr = '';
			xmlstr += '<juan ver="1.0" seq="0">';
		  	xmlstr += '<setup type="write" user="' + This._USR + '" password="' + This._PWD + '">';
		    xmlstr += '<system operation="default factory" />';
		    xmlstr += '</setup>';
		    xmlstr += '</juan>';
	     
		dataType='jsonp';  //数据类型
        jsonp='jsoncallback'; // 回调函数
	 
	    _AJAXget(this.getRequestURL()+'/cgi-bin/gw2.cgi?f=j','xml='+xmlstr,false,'',function(str){

	    	if(str=='loading_success')
	          showAJAXHint('factory_reset_success').fadeOut(2000);
	        else
	          showAJAXHint(str);

    		$('<input value="'+This._ID+'" type="hidden" class="data" id="dev_id_ID" />').appendTo('#confirm');
   
    		$('#RemoveDeviceEx_ok').click();
		});		  
    }
	this.checkMultRequests = function(str){

	    var l = _Request.length-1;
	   
	    RequesePush(str);
		
		if(!_Request[l])
			return;

		for(var i=0;i<_Request.length;i++){
			if(_Request[i]!='loading_success' && _Request[i]!='save_success'){
				showAJAXHint(_Request[i]);
			}else{
				showAJAXHint(_Request[i]).fadeOut(2000);
			}
		}

		function RequesePush(str){
			for(var i=0;i<_Request.length;i++){
				if(!_Request[i]){
					_Request[i]=str;
					return;
				}
			}
		}
	}

	this.getBoolean = function (name){
		var warp = $('#set_content div.ipc_list:visible');
		return warp.find('input[name="'+name+'"]:checked').val()=='true'? true:false;
	}

	this.getVlue=function (name){
		var warp = $('#set_content div.ipc_list:visible');
		return warp.find('input[data-UI="'+name+'"]:checked').val() || warp.find('input[data-UI="'+name+'"]').val()
	}

	this.getData=function (name){
		var warp = $('#set_content div.ipc_list:visible');
		return warp.find('input[data-UI="'+name+'"]:checked').val() || warp.find('input[data-UI="'+name+'"]').attr('data')
	}

	return this;
}