var IPC = function(usr,pwd,url,type){
	this._USR = usr;
	this._PWD = pwd;
	this._URL = url;
	this._VER = 0;

console.log(arguments);
	auth = "Basic " + base64.encode(this._USR+':'+this._PWD);

	/*$(document).ajaxSend(function(re){
		console.log('--------------ajaxSend-------------------------');
		console.log(re);
		re.setRequestHeader('Authorization',auth);
	});*/


	this.ipcBasicInfo2UI = function(){
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
		
		_AJAXget(this._URL+'/cgi-bin/gw2.cgi?f=j','xml='+xmlstr,'',function(data){
			data2UI(data);
			This._VER = $('#set_content div.ipc_list:visible input[data-UI="hardware_version"]').val();
		});
	}

	this.ipcencodeInfo2UI = function(){
		console.log('-------------------ipcencodeInfo2UI--------------------------');
		var This = this;
		_AJAXget(this._URL + '/netsdk/video/encode/channel/'+$('#set_content div.ipc_list:visible input[data]:first').attr('data')+'/properties','',function(){
			$('#set_content div.ipc_list:visible ul.option:eq(0) li').each(function(){
				$(this)[0].onclick=function(){
					_AJAXget(This._URL+'/netsdk/video/encode/channel/'+$(this).find('input').attr('data')+'/properties','','',data2UI);
				}
			})
		},data2UI);
	}

	this.ipcencodeInfoPut = function(){

		var warp = $('#set_content div.ipc_list:visible');
	
		console.log('---------------put----------------------');
		var str = getPutDataJSON().replace('可变码率','VBR');
			str = str.replace('固定码率','CBR');
		console.log(str);

		/*var oJSON = $.parseJSON(getPutDataJSON())
			oJSON.bitRateControlType = warp.find('input[data-UI="bitRateControlType"]').attr('data');
		console.log(json2str(oJSON));*/
		_AJAXput(this._URL + '/netsdk/video/encode/channel/'+warp.find('input[data]:first').attr('data'),str);

	}

	this.ipcnetworkInfo2UI = function(){

		console.log('-------------------ipcnetworkInfo2UI--------------------------');
		var warp = $('#set_content div.ipc_list:visible');

		/*var xmlstr = '';
		xmlstr += '<juan ver="" seq="">';
		xmlstr += '<conf type="read" user="'+this._USR+'" password="'+this._PWD+'">';
		xmlstr += '<network mac="">';
		xmlstr += '<lan dhcp="" static_ip="" static_netmask="" static_gateway="" static_preferred_dns="" static_alternate_dns="" >'
		xmlstr += '<port0 name="" value=""/>'
		xmlstr += '</lan>'
		xmlstr += '<esee enable="" id_disp=""/>';
		xmlstr += '<pppoe enable="" username="" password="" />';
		xmlstr += '<ddns enable="" provider="" url="" username="" password="" />';
		//xmlstr += '<threeg enable="" apn="" pin="" username="" password="" />';
		xmlstr += '</network>';
		xmlstr += '</conf>';
		xmlstr += '</juan>';

		dataType='jsonp';
		jsonp='jsoncallback';
		
		_AJAXget(this._URL+'/cgi-bin/gw2.cgi?f=j','xml=' + xmlstr,'',data2UI);*/

		_AJAXget(this._URL+'/netsdk/System/deviceInfo/macAddress','','',function(data){
			warp.find('input[data-UI="mac"]').val(data);
		});

		_AJAXget(this._URL+'/netsdk/Network/Interface/1','','',data2UI);

		_AJAXget(this._URL+'/netsdk/Network/Esee','','',function(data){
			warp.find('input[data-WARP="esee"] input[value="'+data.enabled+'"]').prop('checked',true);
		});

		_AJAXget(this._URL+'/netsdk/Network/Dns','','',data2UI);

		_AJAXget(this._URL+'/netsdk/Network/Port/1','','',function(data){
			warp.find('input[data-UI="value"]').val(data.value);
			warp.find('[data-WARP="lan"] [data-UI="addressingType"]:checked').val() == 'dynamic' && disable('lan',true);
			warp.find('[data-WARP="ddns"] [data-UI="enabled"]:checked').val() == 'true' && disable('ddns',true);
			warp.find('[data-WARP="pppoe"] [data-UI="enabled"]:checked').val() == 'true' && disable('pppoe',true);
		});

	}

	this.ipcnetworkInfoPut = function(){

		var warp = $('#set_content div.ipc_list:visible');

		_AJAXput(this._URL+'/netsdk/Network/Interface/1','{"lan":{"addressingType":"'+warp.find('input[data-UI="addressingType"]:checked').val()+'", "staticIP": "'+warp.find('input[data-UI="staticIP"]').val()+'","staticNetmask": "'+warp.find('input[data-UI="staticNetmask"]').val()+'", "staticGateway": "'+warp.find('input[data-UI="staticGateway"]').val()+'" }}');

		_AJAXput(this._URL+'/netsdk/Network/Esee','{"enabled":'+(warp.find('input[data-warp="esee"] input:checked').val()=='true'? true:false)+'}');

		_AJAXput(this._URL+'/netsdk/Network/Dns','{ "preferredDns": "'+warp.find('input[data-UI="preferredDns"]').val()+'", "staticAlternateDns": "'+warp.find('input[data-UI="staticAlternateDns"]').val()+'" }');

		_AJAXput(this._URL+'/netsdk/Network/Port/1','{ "value": "'+warp.find('input[data-UI="value"]').val()+'"}');
	}

	this.ipcuserManagementInfo2UI = function(){

		dataType='';

		console.log('-------------------ipcuserManagementInfo2UI--------------------------');
		
		_AJAXget(this._URL+'/user/user_list.xml','username='+this._USR+'&password='+this._PWD,'',function(data){
			$('user_list',data).children().not(':first').each(function(){
				/*<user0 name=​"user" admin=​"yes" permit_live=​"yes" permit_setting=​"yes" permit_playback=​"yes" del_user=​"yes" edit_user=​"no" set_pass=​"no">​</user0>​*/
				console.log($(this));
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

	this.ipczoneInfo2UI = function(){

		$('#PC_time').val(renewtime());

		console.log('-------------------timeZone--------------------------');
		_AJAXget(this._URL+'/Netsdk/system/time/timeZone','','',function(data){
			$('#time_zone').val(data).attr('data',data);
		});

		console.log('-------------------ntp-------------'+this._URL+'/netsdk/system/time/ntp-------------');
		_AJAXget(this._URL+'/Netsdk/system/time/ntp','','',data2UI);

		console.log('-------------------localTime-------------'+this._URL+'/NetSDK/System/time/localTime-------------');
		_AJAXget(this._URL+'/NetSDK/System/time/localTime','','',function(data){
			var time = data.split('+')[0].replace('T','  ');
			$('#curent_time').val(time).attr('data',time);
			//console.log(data);
		})

		console.log('-------------------PCTime--------------------------');

		setInterval(function(){
			$('#PC_time').val(renewtime());
		},1000);

		function renewtime(){
			var myDate = new Date,

				yy=myDate.getFullYear(),
			/*if (yy.length<4)
			{
				var i = 4-yy.length;
				for (var j = 0; j < i; j++)
				{
					yy = "0" + yy;
				}
			}*/
			mm=addZero(parseInt(myDate.getMonth())+1),

			dd=addZero(myDate.getDate()),

			hh=addZero(myDate.getHours()),

			mi=addZero(myDate.getMinutes()),

			ss=addZero(myDate.getSeconds());

			return yy + "-" + mm + "-" + dd + "  " + hh + ":" + mi + ":" + ss;
		}

		this.ipczoneInfoPut = function(){

			var warp = $('#set_content div.ipc_list:visible');

			_AJAXput(this._URL+'/netsdk/system/time/timeZone','"'+$('#time_zone').val()+'"');

			_AJAXput(this._URL+'/netsdk/system/time/ntp','{"ntpEnabled":'+(warp.find('input[data-UI="ntpEnabled"]:checked').val() == 'true' ? true : false)+',"ntpServerDomain":"'+warp.find('input[data-UI="ntpServerDomain"]').val()+'"}')
		}
	}

	return this;
}