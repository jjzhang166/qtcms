var IPC = function(usr,pwd,ip,port,id,type){
	this._IP = ip;
	this._PORT = port;
	this._USR = usr;
	this._PWD = pwd;
	this._ID = id;
	this._TYPE = type;
	this._VER = 0;
	this._Upgrade = '1.3.0';  // CMS 支持的最低版本IPC

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
		var warp = $('#set_content div.ipc_list:visible');
		_AJAXget(this.getRequestURL() + '/netsdk/video/encode/channel/'+num+'/properties','','',
			function(data){
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

		var warp = $('#set_content div.ipc_list:visible');
	
		console.log('---------------put----------------------');
		var str = getPutDataJSON().replace('可变码率','VBR');
			str = str.replace('固定码率','CBR');
		//console.log(str);

		/*var oJSON = $.parseJSON(getPutDataJSON())
			oJSON.bitRateControlType = warp.find('input[data-UI="bitRateControlType"]').attr('data');
		console.log(json2str(oJSON));*/
		_AJAXput(this.getRequestURL() + '/netsdk/video/encode/channel/'+warp.find('input[data]:first').attr('data'),str);

	}

	this.ipcnetworkInfo2UI = function(){ //获取网络信息

		emptyDevSetMenu();
		
		//async=false;

		console.log('-------------------ipcnetworkInfo2UI--------------------------');
		var warp = $('#set_content div.ipc_list:visible');

		_AJAXget(this.getRequestURL()+'/netsdk/System/deviceInfo/macAddress','','',function(data){
			warp.find('input[data-UI="mac"]').val(data);
		});

		_AJAXget(this.getRequestURL()+'/netsdk/Network/Interface/1','','',function(data){
			data2UI(data);
			warp.find('[data-WARP="lan"] [data-UI="addressingType"]:checked').val() == 'dynamic' ? disable('lan',true) : disable('lan');
			warp.find('[data-WARP="ddns"] [data-UI="enabled"]:checked').val() == 'true' ? disable('ddns') : disable('ddns',true);
			warp.find('[data-WARP="pppoe"] [data-UI="enabled"]:checked').val() == 'true' ? disable('pppoe') : disable('pppoe',true);
		});

		_AJAXget(this.getRequestURL()+'/netsdk/Network/Esee','','',function(data){
			warp.find('input[name="esee"][value="'+data.enabled+'"]').prop('checked',true);
		});

		_AJAXget(this.getRequestURL()+'/netsdk/Network/Dns','','',data2UI);

		_AJAXget(this.getRequestURL()+'/netsdk/Network/Port/1','','',function(data){
			warp.find('input[data-UI="value"]').val(data.value);
		});

		//async=true;
	}

	this.ipcnetworkInfoPut = function(){ //设置网络信息

		async=false;

		var warp = $('#set_content div.ipc_list:visible'),
			This = this,

			interFaceJSON = '{"lan":{"addressingType":"'+getVlue('addressingType')+'", "staticIP": "'+getVlue('staticIP')+'","staticNetmask": "'+getVlue('staticNetmask')+'", "staticGateway": "'+getVlue('staticGateway')+'" },"pppoe": { "enabled": '+getBoolean('pppoe')+', "pppoeUserName": "'+getVlue('pppoeUserName')+'", "pppoePassword": "'+getVlue('pppoePassword')+'" }, "ddns": { "enabled": '+getBoolean('ddns')+', "ddnsProvider": "'+getVlue('ddnsProvider')+'", "ddnsUrl": "'+getVlue('ddnsUrl')+'", "ddnsUserName": "'+getVlue('ddnsUserName')+'", "ddnsPassword": "'+getVlue('ddnsPassword')+'" }}';

		_AJAXput(this.getRequestURL()+'/netsdk/Network/Esee','{"enabled":'+(getBoolean('esee'))+'}'/*,'','',function(){
			console.log('Esee状态修改完成{"enabled":'+(warp.find('input[data-warp="esee"] input:checked').val()=='true'? true:false)+'}++++++++++2');
		}*/);

		_AJAXput(this.getRequestURL()+'/netsdk/Network/Dns','{"preferredDns": "'+warp.find('input[data-UI="preferredDns"]').val()+'", "staticAlternateDns": "'+warp.find('input[data-UI="staticAlternateDns"]').val()+'" }'/*,'','',function(){
			console.log('Dns状态修改完成++++++++++3');
		}*/);

		_AJAXput(this.getRequestURL()+'/netsdk/Network/Port/1','{ "value": '+warp.find('input[data-UI="value"]').val()+'}'/*,'',function(){
			This._PORT = warp.find('input[data-UI="value"]').val();
			console.log('端口修改成功+++++++++++++++++++++++++4'+This.getRequestURL());
		}*/);

		_AJAXput(this.getRequestURL()+'/netsdk/Network/Interface/1',interFaceJSON,'',function(){
			//This._IP = warp.find('input[data-UI="staticIP"]').val();
			qob.OnModifyDeviceEx();
			/*console.log('--修改端口和地址重新同步设备状态----');
			areaList2Ui();
			console.log('--修改端口和地址重新加载设备列表----');*/
			reInitNowDev();
			//console.log('IP修改成功++1'+This.getRequestURL());
		});

		function getBoolean(name){
			return warp.find('input[name="'+name+'"]:checked').val()=='true'? true:false
		}

		function getVlue(name){
			return warp.find('input[data-UI="'+name+'"]:checked').val() || warp.find('input[data-UI="'+name+'"]').val()
		}

		async=true;
	}

	this.ipcuserManagementInfo2UI = function(){ //获取用户信息

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

		$('#PC_time').val(renewtime());

		console.log('-------------------timeZone--------------------------');
		_AJAXget(this.getRequestURL()+'/Netsdk/system/time/timeZone','','',function(data){
			$('#time_zone').val(data).attr('data',data);
		});

		//console.log('-------------------ntp-------------'+this.getRequestURL()+'/netsdk/system/time/ntp-------------');
		_AJAXget(this.getRequestURL()+'/Netsdk/system/time/ntp','','',data2UI);

		//console.log('-------------------localTime-------------'+this.getRequestURL()+'/NetSDK/System/time/localTime-------------');
		_AJAXget(this.getRequestURL()+'/NetSDK/System/time/localTime','','',function(data){
			var time = data.split('+')[0].replace('T','  ');
			$('#curent_time').val(time).attr('data',time);
			//console.log(data);
		})

		//console.log('-------------------PCTime--------------------------');

		setInterval(function(){
			$('#PC_time').val(renewtime());
		},1000);

		this.ipczoneInfoPut = function(){ //设置时区

			var warp = $('#set_content div.ipc_list:visible');

			_AJAXput(this.getRequestURL()+'/netsdk/system/time/timeZone','"'+$('#time_zone').val()+'"');

			_AJAXput(this.getRequestURL()+'/netsdk/system/time/ntp','{"ntpEnabled":'+(warp.find('input[data-UI="ntpEnabled"]:checked').val() == 'true' ? true : false)+',"ntpServerDomain":"'+warp.find('input[data-UI="ntpServerDomain"]').val()+'"}')
		}
	}

	return this;
}