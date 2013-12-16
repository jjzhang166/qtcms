var ipc_url,g_usr,g_pwd;
var yy,mm,dd,hh,mi,ss;
var init_set_time = false;
var base64 = new Base64();

function ipc(_url,_usr,_pwd){
ipc_url = _url;
g_usr = _usr;
g_pwd = _pwd;
}
$(window).ready(function(){
//开启本地时间
	setInterval(renewtime,1000)
	$('.in').each(function(index){
			$(this).focusout(function() {
				if($(this).val() == ''){
					alert('输入不能为空');
					}
			});
			$(this).focus(function(){
				$(this).val('');
			});
			$(this).keyup(function(){
				var str = $(this).val();
				if(index == 0){
					if(str.length == 1){
						if(str > 2){
							$(this).val('2');	
						}	
					}else{
						if(str.slice(0,1) == '2'){
							if(str.slice(1,2) > 4){
								$(this).val(str.slice(0,1)+'3');
							}
						}
					}
				}else{
					if(str.length == 1){
						if(str > 6){
							$(this).val('5');
						}	
					}	
				}
				if($(this).val().length == 2){
					$('.in').eq(index + 1).focus();	
				}
			});	
})
//本地时间函数
	function renewtime()
	{
		var myDate = new Date;
		yy=myDate.getFullYear().toString();
		if (yy.length<4)
		{
			var i = 4-yy.length;
			for (var j = 0; j < i; j++)
			{
				yy = "0" + yy;
			}
		}
		mm=(myDate.getMonth()+parseInt(1)).toString();
		mm=(mm.length==1)?("0"+mm):mm
		dd=myDate.getDate().toString();
		dd=(dd.length==1)?("0"+dd):dd
		hh=myDate.getHours().toString();
		hh=(hh.length==1)?("0"+hh):hh
		mi=myDate.getMinutes().toString();
		mi=(mi.length==1)?("0"+mi):mi
		ss=myDate.getSeconds().toString();
		ss=(ss.length==1)?("0"+ss):ss
		var str = yy + "-" + mm + "-" + dd + "  " + hh + ":" + mi + ":" + ss;
		$('#time_pc').val('').val(str);
		if(!init_set_time){
			$('.m_date').val(yy + "-" + mm + "-" + dd);
			$('.in').eq(0).val(hh).end().eq(1).val(mi).end().eq(2).val(ss);
			init_set_time = true;
		}
	}
});	

//encode
function check(){
	for(i=0;i<document.getElementsByName("channelName")[0].value.length;i++){
		var c = document.getElementsByName("channelName")[0].value.substr(i,1);
		var ts = escape(c);
		if(ts.substring(0,2) == "%u"){
		alert(langstr.can_not_input_Chinese);
		document.getElementsByName("channelName")[0].value = "";
		document.getElementsByName("channelName")[0].focus();
		}
	}
}
function deviceName_load()
{
	//var auth = "Basic " + base64.encode(g_usr+':'+g_pwd);
	var auth = "Basic " + base64.encode('admin:');
		$.ajax({
				type:"GET",
				url:ipc_url + '/netsdk/system/deviceinfo',
				dataType:"json",
				beforeSend : function(req){ 
				req .setRequestHeader('Authorization', auth);
				},
				success:function(data){	
					$('#ipc_enc_channelname').val(data.deviceName);
				},
				error:function(a,b,c){ 
					alert(b);
				}
			});
}
function encode_load_content()
{
	//var auth = "Basic " + base64.encode(g_usr+':'+g_pwd);
	var auth = "Basic " + base64.encode('admin:');
	var id = $('#ipc_enc_stream').html();
		$.ajax({
				type:"GET",
				url:ipc_url + '/netsdk/video/encode/channel/'+id+'/properties',
				dataType:"json",
				beforeSend : function(req){ 
				req .setRequestHeader('Authorization', auth);
				},
				success:function(data){
					//alert(11)
					$('#ipc_enc_resolution_sel').find('li').remove();
					for(i=0;i<data.resolutionProperty.opt.length;i++)
					{
						$('#ipc_enc_resolution_sel').append('<li class="add_li_2"><a href="javascript:;">'+data.resolutionProperty.opt[i]+'</a></li>');
					}
					encode_load();
				},
				error:function(a,b,c){ 
					alert(b);
				}
			});
}
function encode_load()
{
	//var auth = "Basic " + base64.encode(g_usr+':'+g_pwd);
	var auth = "Basic " + base64.encode('admin:');
	var id = $('#ipc_enc_stream').html();
	$.ajax({
			type:"GET",
			url:ipc_url + '/netsdk/video/encode/channel/'+ id,
			dataType:"json",
			beforeSend : function(req){ 
        	req .setRequestHeader('Authorization', auth);
    		},
			success:function(data){
				free_resolution = data.freeResolution;
				encode_data2ui();		
				$('#ipc_enc_channelname').val(data.channelName);
				$('#ipc_enc_resolution').html(data.resolution);
				$('#ipc_enc_resolutionWidth').val(data.resolutionWidth);
				$('#ipc_enc_resolutionHeight').val(data.resolutionHeight);
				$('#ipc_enc_BitRateControlType').html(data.bitRateControlType);
				$('#ipc_enc_bps').val(data.constantBitRate);
				$('#ipc_enc_fps').val(data.frameRate);
				$('#ipc_enc_freeResolution')[0].onclick = function(){
					var obj = $('#ipc_enc_freeResolution_0');
					var obj1 = $('#ipc_enc_freeResolution_1');
						obj.is(':visible') ? obj.hide() : obj.show();
						obj1.is(':visible') ? obj1.hide() : obj1.show();
				}
			}
		});
}
function encode_data2ui()
{
		if(free_resolution == true){
			$('#ipc_enc_freeResolution').prop('checked',true);
			$('#ipc_enc_freeResolution_0')[0].style.display='none';
			$('#ipc_enc_freeResolution_1').removeAttr("style");
			//$('#overlay_freeresolution_1')[0].style.display='inline-block';
		}else{
			$('#ipc_enc_freeResolution').prop('checked',false);
			$('#ipc_enc_freeResolution_0').removeAttr("style");
			//$('#overlay_freeresolution_0')[0].style.display='inline-block';
			$('#ipc_enc_freeResolution_1')[0].style.display='none';	
		}
		switch($('#id_freeresolution_1').val())
		{
			case 'true': $('#id_freeresolution_1')[0].checked = 1; break;
			case 'false': $('#id_freeresolution_0')[0].checked = 1; break;
			default: break;
		}
		$('#ipc_enc_stream_sel li a').each(function(index) {
			$(this).click(function(){
						$('#ipc_enc_stream').html($(this)[0].innerHTML);
						encode_load_content();
				})
		});
}
function encode_save_content()
{	
	var id = $('#ipc_enc_stream').html();
	var ipc_enc_channelname = $('.ipc_enc_channelname').val();
	var ipc_enc_resolution =$('#ipc_enc_resolution').html();
	var ipc_enc_BitRateControlType =$('#ipc_enc_BitRateControlType').html();
	var ipc_enc_freeResolution;
	var ipc_enc_resolutionWidth = $('#ipc_enc_resolutionWidth').val();
	var ipc_enc_resolutionHeight = $('#ipc_enc_resolutionHeight').val();
	var ipc_enc_bps = $('#ipc_enc_bps').val();
	var ipc_enc_fps = $('#ipc_enc_fps').val();
	if($('input#ipc_enc_freeResolution:checked').val() == 'on')
	{
		ipc_enc_freeResolution = true;
	}else
	{
		ipc_enc_freeResolution = false;
	}
	var encode_data = '{"channelName": "'+ ipc_enc_channelname +'","resolution":"'+ipc_enc_resolution+'","freeResolution":'+ipc_enc_freeResolution+',"resolutionWidth":'+ ipc_enc_resolutionWidth +',"resolutionHeight":'+ ipc_enc_resolutionHeight +',"bitRateControlType":"'+ipc_enc_BitRateControlType+'","constantBitRate":'+ ipc_enc_bps +',"frameRate":'+ ipc_enc_fps +'}';
	
	//	alert(encode_data); 
	 
	//var auth = "Basic " + base64.encode(g_usr+':'+g_pwd);
	var auth = "Basic " + base64.encode('admin:');
	$.ajax({
			type:'PUT',
			url:ipc_url + '/netsdk/video/encode/channel/'+ id,
			dataType:'json',
			data:encode_data,
			async:false,
			beforeSend : function(req ) {
        	req .setRequestHeader('Authorization', auth);
    		},
			success:function(data){
			},
			error:function(a,b,c){ 
				alert(b);
			}
		})
}
//devinfo
function devinfo_load_content(bflag)
{
	var xmlstr = '';
	xmlstr += '<juan ver="" seq="">';
	xmlstr += '<conf type="read" user="admin" password="">';
	xmlstr += '<spec vin="" ain="" io_sensor="" io_alarm="" hdd="" sd_card="" />';
	xmlstr += '<info device_name="" device_model="" device_soc="" device_sn="" sensor_type="" hardware_version="" software_version="" build_date="" build_time="" />';
	xmlstr += '</conf>';
	xmlstr += '</juan>';
//	alert(xmlstr);
	dvr_ajax = $.ajax({ 
		type:"GET",
		url: ipc_url + "/cgi-bin/gw2.cgi?f=j", 
		processData: false, 
		cache: false,
		data: "xml=" + xmlstr, 
		async:true,
		dataType: 'jsonp',
		jsonp: 'jsoncallback',
		beforeSend: function(XMLHttpRequest){
		},
		success: function(data, textStatus){
//		alert("recv:" + data);
			var dvr_data = xml2json.parser(data.xml, "", false);
			$("#Ipc_Device_name")[0].value = dvr_data.juan.conf.info.device_name;
			$("#Ipc_Device_model")[0].value = dvr_data.juan.conf.info.device_model;
			$("#Ipc_Hardware_version")[0].value = dvr_data.juan.conf.info.hardware_version;
			$("#Ipc_Software_version")[0].value = dvr_data.juan.conf.info.software_version;
			$("#Ipc_Build_time")[0].value = dvr_data.juan.conf.info.build_date + " " + dvr_data.juan.conf.info.build_time;
			$("#Ipc_Alarm_numbers")[0].value = dvr_data.juan.conf.spec.io_alarm;
			$("#Ipc_SD_numbers")[0].value = dvr_data.juan.conf.spec.sd_card;				
			
		},
		complete: function(XMLHttpRequest, textStatus){
//			alert("complete:" + textStatus);
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
//			alert("error:" + textStatus);
		}
	});	
}
//network
function network_data2ui(dvr_data)
{
	switch (dvr_data.juan.conf.network.lan.dhcp)
	{
		case "yes": $("#network_dhcp_1")[0].checked = 1;break;
		case "no": $("#network_dhcp_0")[0].checked = 1;break;
		default:break;	
	};
	switch (dvr_data.juan.conf.network.esee.enable)
	{
		case "yes": $("#network_esee_1")[0].checked = 1;break;
		case "no": $("#network_esee_0")[0].checked = 1;break;
		default:break;	
	};
	$("#network_mac")[0].value = dvr_data.juan.conf.network.mac;
	$("#network_ip")[0].value = dvr_data.juan.conf.network.lan.static_ip;
	$("#network_gateway")[0].value = dvr_data.juan.conf.network.lan.static_gateway;
	$("#network_submask")[0].value = dvr_data.juan.conf.network.lan.static_netmask;
	$("#network_dns")[0].value = dvr_data.juan.conf.network.lan.static_preferred_dns;
	$("#network_dns2")[0].value = dvr_data.juan.conf.network.lan.static_alternate_dns;
	$("#network_port")[0].value = dvr_data.juan.conf.network.lan.port0.value;
	ip_config_change();
}
function ip_config_change()
{
	if($("#network_dhcp_1")[0].checked ==1)
	{
		//alert("use dhcp");
		$("#network_ip")[0].disabled = true;
		$("#network_gateway")[0].disabled = true;
		$("#network_submask")[0].disabled = true;
		$("#network_dns")[0].disabled = true;
		$("#network_dns2")[0].disabled = true;
		$("#network_port")[0].disabled = true;
	}
	else
	{
		//alert("use static ip");
		$("#network_ip")[0].disabled = false;
		$("#network_gateway")[0].disabled = false;
		$("#network_submask")[0].disabled = false;
		$("#network_dns")[0].disabled = false;
		$("#network_dns2")[0].disabled = false;
		$("#network_port")[0].disabled = false;
	}
}
function network_load_content()
{
	var xml = '';
	xml += '<juan ver="" seq="">';
	xml += '<conf type="read" user="admin" password="">';
	xml += '<network mac="">';
	xml += '<lan dhcp="" static_ip="" static_netmask="" static_gateway="" static_preferred_dns="" static_alternate_dns="" >'
	xml += '<port0 name="" value=""/>'
	xml += '</lan>'
	xml += '<esee enable="" id_disp=""/>';
	xml += '</network>';
	xml += '</conf>';
	xml += '</juan>';
//	alert(xmlstr);

	dvr_ajax = $.ajax({ 
		type:"GET",
		url: ipc_url + "/cgi-bin/gw2.cgi?f=j", 
		processData: false, 
		cache: false,
		data: "xml=" + xml, 
		async:true,
		dataType: 'jsonp',
		jsonp: 'jsoncallback',

		beforeSend: function(XMLHttpRequest){
		},
		success: function(data, textStatus){
//			alert("recv:" + data.xml);
			var dvr_data = xml2json.parser(data.xml, "", false);
			network_data2ui(dvr_data);
			remote_load_content();
		},
		complete: function(XMLHttpRequest, textStatus){
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
			//alert("error:" + textStatus);
		}
	});	
}
function network_save_content()
{
	var dhcp_s,esee_s;
	switch ($("#network_dhcp_1")[0].checked)
	{
		case true: dhcp_s = "yes";break;
		case false: dhcp_s = "no";break;
		default: break;	
	};
	switch ($("#network_esee_1")[0].checked)
	{
		case true: esee_s = "yes";break;
		case false: esee_s = "no";break;
		default:break;	
	};
	var xml = '';
	xml += '<juan ver="1.0" seq="0">';
	xml += '<conf type="write" user="admin" password="">';
	xml += '<network mac="' + $("#network_mac")[0].value + '">';
	xml += '<lan ';
	xml += 'dhcp="' + dhcp_s + '" ';
	xml += 'static_ip="' + $("#network_ip")[0].value + '" ';
	xml += 'static_netmask="' + $("#network_submask")[0].value + '" ';
	xml += 'static_gateway="' + $("#network_gateway")[0].value + '" ';
	xml += 'static_preferred_dns="' + $("#network_dns")[0].value + '" ';
	xml += 'static_alternate_dns="' + $("#network_dns2")[0].value + '" >';
	xml += '<port0 name="generic" value="' + $("#network_port")[0].value + '"/>';
	xml += '</lan>';
	xml += '<esee enable="' + esee_s + '"/>';
	xml += '</network>';
	xml += '</conf>';
	xml += '</juan>';
//	alert(xml);

	dvr_ajax = $.ajax({ 
		type:"GET",
		url: ipc_url + "/cgi-bin/gw2.cgi?f=j", 
		processData: false, 
		cache: false,
		data: "xml=" + xml, 
		async:true,
		dataType: 'jsonp',
		jsonp: 'jsoncallback',

		beforeSend: function(XMLHttpRequest){
		},
		success: function(data, textStatus){
		//	alert("recv:" + data.xml);
		},
		complete: function(XMLHttpRequest, textStatus){
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
			//alert("error:" + textStatus);
		}
	});	
	remote_save_content();
}
//remote
function remote_data2ui(dvr_data)
{
	$("#remote_provider")[0].innerHTML = dvr_data.juan.conf.network.ddns.provider;
	$("#networkddns_url")[0].value = dvr_data.juan.conf.network.ddns.url;
	$("#networkddns_usr")[0].value = dvr_data.juan.conf.network.ddns.username;
	$("#networkddns_pwd")[0].value = dvr_data.juan.conf.network.ddns.password;
	$("#network_ppoe_usr")[0].value = dvr_data.juan.conf.network.pppoe.username;
	$("#network_ppoe_pwd")[0].value = dvr_data.juan.conf.network.pppoe.password;

	switch (dvr_data.juan.conf.network.ddns.enable)
	{
		case "yes": $("#network_ddns_1")[0].checked = 1;break;
		case "no": $("#network_ddns_0")[0].checked = 1;break;
		default: break;
	}
	switch (dvr_data.juan.conf.network.pppoe.enable)
	{
		case "no": $("#network_pppoe__0")[0].checked = 1;break;
		case "yes": $("#network_pppoe__1")[0].checked = 1;break;
		default: break;
	}
	remote_change();
}

function remote_load_content()
{
	var xmlstr = '';
	xmlstr += '<juan ver="" seq="">';
	xmlstr += '<conf type="read" user="admin" password="">';
	xmlstr += '<network>';
	xmlstr += '<pppoe enable="" username="" password="" />';
	xmlstr += '<ddns enable="" provider="" url="" username="" password="" />';
	xmlstr += '<threeg enable="" apn="" pin="" username="" password="" />';
	xmlstr += '</network>';
	xmlstr += '</conf>';
	xmlstr += '</juan>';
//	alert(xmlstr);

	dvr_ajax = $.ajax({ 
		type:"GET",
		url: ipc_url + "/cgi-bin/gw2.cgi?f=j", 
		processData: false, 
		cache: false,
		data: "xml=" + xmlstr, 
		async:true,
		dataType: 'jsonp',
		jsonp: 'jsoncallback',

		beforeSend: function(XMLHttpRequest){
		},
		success: function(data, textStatus){
//			alert("recv:" + data.xml);
			var dvr_data = xml2json.parser(data.xml, "", false);
			remote_data2ui(dvr_data);
		},
		complete: function(XMLHttpRequest, textStatus){
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
			//alert("error:" + textStatus);
		}
	});	
}
		
function remote_save_content()
{
	var ddns_s,pppoe_s; 
	switch ($("#network_ddns_1")[0].checked)
	{
		case true: ddns_s = "yes";break;
		case false: ddns_s = "no";break;
		default: break;
	}
	switch ($("#network_pppoe__1")[0].checked)
	{
		case true: pppoe_s = "yes";break;
		case false: pppoe_s = "no";break;
		default: break;
	}
	var provider = $("#remote_provider")[0].innerHTML;

	var xmlstr = '';
	xmlstr += '<juan ver="1.0" seq="0">';
	xmlstr += '<conf type="write" user="admin" password="">';
	xmlstr += '<network>';
	xmlstr += '<pppoe enable="' + pppoe_s + '" username="' + $("#network_ppoe_usr")[0].value + '" password="' + $("#network_ppoe_pwd")[0].value + '" />';
	xmlstr += '<ddns enable="' + ddns_s + '" provider="'+ provider +'" url="' + $("#networkddns_url")[0].value + '" username="' + $("#networkddns_usr")[0].value + '" password="' + $("#networkddns_pwd")[0].value + '" />';
	xmlstr += '</network>';
	xmlstr += '</conf>';
	xmlstr += '</juan>';
	//alert(xmlstr);

	dvr_ajax = $.ajax({ 
		type:"GET",
		url: ipc_url + "/cgi-bin/gw2.cgi?f=j", 
		processData: false, 
		cache: false,
		data: "xml=" + xmlstr, 
		async:true,
		dataType: 'jsonp',
		jsonp: 'jsoncallback',

		beforeSend: function(XMLHttpRequest){
		},
		success: function(data, textStatus){
			//alert("recv:" + data.xml);
		network_save_content();
		},
		complete: function(XMLHttpRequest, textStatus){
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
			//alert("error:" + textStatus);
		}
	});	
}

function remote_change()
{
	if($("#network_ddns_1")[0].checked == 1)
	{
		$("#remote_provider")[0].disabled = false;			
		$("#networkddns_url")[0].disabled = false;			
		$("#networkddns_usr")[0].disabled = false;			
		$("#networkddns_pwd")[0].disabled = false;			
	}
	else
	{
		$("#remote_provider")[0].disabled = true;			
		$("#networkddns_url")[0].disabled = true;			
		$("#networkddns_usr")[0].disabled = true;			
		$("#networkddns_pwd")[0].disabled = true;			
	}

	if($("#network_pppoe__1")[0].checked == 1)
	{
		$("#network_ppoe_usr")[0].disabled = false;			
		$("#network_ppoe_pwd")[0].disabled = false;			
	}
	else
	{
		$("#network_ppoe_usr")[0].disabled = true;			
		$("#network_ppoe_pwd")[0].disabled = true;			
	}
}
//user_management

var user_management_target = "";
function user_management_prepare_rm()
{
	$("#tbl_add_user")[0].style.display = "none";
	$("#tbl_modify_pwd")[0].style.display = "none";
	if(confirm('Are you sure to delete it?'))
	{
		user_management_save_del_usr();
	}
}
function user_management_save_del_usr()
{
	var xmlstr = "";
	xmlstr += "<user>";
	xmlstr += "<del_user name=\"" + user_management_target + "\" />";
	xmlstr += "</user>";
//	alert(xmlstr);

	dvr_ajax = $.ajax({ 
		type:"GET",
		url: ipc_url + "/user/del_user.xml", 
		processData: false, 
		cache: false,
		data: "username=" + g_usr + "&password=" + g_pwd + "&content=" + xmlstr, 
		async:true,

		beforeSend: function(XMLHttpRequest){
//			alert("beforeSend");
		},
		success: function(data, textStatus, xmlhttp){
//			alert("recv:" + data.xml);
			var dvr_data = xml2json.parser(xmlhttp.responseText, "", false);
			if(dvr_data.user.ret != "success")
			{
				alert('delete_fail');	
			}
			else
			{
				user_management_load_content();
			}
			user_management_target = "";
		},
		complete: function(XMLHttpRequest, textStatus){
//			alert("complete:" + textStatus);
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
			alert("error:" + textStatus);
		}
	});	
}

function user_management_save_edit_usr()
{
	alert(document.getElementById('permit_admin').checked);
	var xmlstr = "";
	xmlstr += "<user>";
	xmlstr += "<edit_user name=\"" + user_management_target  + "\" admin=\"" + + "\" premit_live=\"\" premit_setting=\"" + + "\" premit_playback=\"" + + "\" />";
	xmlstr += "</user>";
//	alert(xmlstr);

	dvr_ajax = $.ajax({ 
		type:"GET",
		url: ipc_url + "/user/add_user.xml", 
		processData: false, 
		cache: false,
		data: "username=" + g_usr + "&password=" + g_pwd + "&content=" + xmlstr, 
		async:true,

		beforeSend: function(XMLHttpRequest){
//			alert("beforeSend");
		},
		success: function(data, textStatus, xmlhttp){
//			alert("recv:" + data.xml);
			var dvr_data = xml2json.parser(xmlhttp.responseText, "", false);
			if(dvr_data.user.ret != "success")
			{
				alert('add_fail');	
			}
			else
			{
				user_management_load_content();
			}
			user_management_target = "";
		},
		complete: function(XMLHttpRequest, textStatus){
//			alert("complete:" + textStatus);
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
			alert("error:" + textStatus);
		}
	});	
}

function user_management_prepare_add()
{
	$("#tbl_add_user")[0].style.display = "block";
	$("#tbl_modify_pwd")[0].style.display = "none";
}


function user_management_save_new_usr()
{
	var use = document.getElementById("txt_new_usr").value;
	if(use==null || use=="")
	{
		alert('warning');
		document.getElementById('username').focus();

		return false;
		}	
	
	$("#tbl_add_user")[0].style.display = "none";
	$("#tbl_modify_pwd")[0].style.display = "none";

	user_management_target = $("#txt_new_usr")[0].value;
	//alert(user_management_target);
	var xmlstr = "";
	xmlstr += "<user>";
	xmlstr += "<add_user name=\"" + $("#txt_new_usr")[0].value + "\" password=\"" + $("#txt_new_pwd")[0].value + "\" admin=\"\" premit_live=\"\" premit_setting=\"\" premit_playback=\"\" />";
	xmlstr += "</user>";
//	alert(xmlstr);

	dvr_ajax = $.ajax({ 
		type:"GET",
		url: ipc_url + "/user/add_user.xml", 
		processData: false, 
		cache: false,
		data: "username=" + g_usr + "&password=" + g_pwd + "&content=" + xmlstr, 
		async:true,

		beforeSend: function(XMLHttpRequest){
//			alert("beforeSend");
		},
		success: function(data, textStatus, xmlhttp){
//			alert("recv:" + data.xml);
			var dvr_data = xml2json.parser(xmlhttp.responseText, "", false);
			if(dvr_data.user.ret != "success")
			{
				alert('add_fail');	
			}
			else
			{
				user_management_load_content();
			}

			user_management_target = "";
		},
		complete: function(XMLHttpRequest, textStatus){
//			alert("complete:" + textStatus);
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
			alert("error:" + textStatus);
		}
	});	
}
function user_management_prepare_modify()
{
	$("#tbl_add_user")[0].style.display = "none";
	$("#tbl_modify_pwd")[0].style.display = "block";
}
function user_management_prepare_save_modify_usr()
{
	if($("#txt_old_pwd")[0].value != g_pwd)
	{
		alert('old_pwd_wrong');
		return;
	}
	if($("#txt_modify_pwd")[0].value != $("#txt_repeat_pwd")[0].value)
	{
		alert('confirm_pwd_wrong');
		return;
	}

	$("#tbl_add_user")[0].style.display = "none";
	$("#tbl_modify_pwd")[0].style.display = "none";

	user_management_dvr_target = g_usr;

	var xmlstr = "";
	xmlstr += "<user>";
	xmlstr += "<set_pass old_pass=\"" + $("#txt_old_pwd")[0].value + "\" new_pass=\"" + $("#txt_modify_pwd")[0].value + "\" />";
	xmlstr += "</user>";

	dvr_ajax = $.ajax({ 
		type:"GET",
		url: ipc_url + "/user/set_pass.xml", 
		processData: false, 
		cache: false,
		data: "username=" + g_usr + "&password=" + g_pwd + "&content=" + xmlstr, 
		async:true,

		beforeSend: function(XMLHttpRequest){
//			alert("beforeSend");
		},
		success: function(data, textStatus, xmlhttp){
//			alert("recv:" + data.xml);
			var dvr_data = xml2json.parser(xmlhttp.responseText, "", false);
			if(dvr_data.user.ret != "success")
			{
				alert('modify_pwd_fail');	
			}
			else{
				alert('modify_success');	
			}

			user_management_target = "";
		},
		complete: function(XMLHttpRequest, textStatus){
//			alert("complete:" + textStatus);
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
			alert("error:" + textStatus);
		}
	});	
}
function user_management_prepare_cancel()
{
	$("#tbl_add_user")[0].style.display = "none";
	$("#tbl_modify_pwd")[0].style.display = "none";
	
	user_management_target = "";
}

function user_management_data2ui(dvr_data)
{
	var user_count = dvr_data.user.user_list.count;
	var tbl = $("#tbl_user_manage")[0];
	for(var i = tbl.rows.length - 1; i >= 2; i--)
	{
		tbl.deleteRow(i);
	}
	if(eval("dvr_data.user.add_user") == "no")
	{
		document.getElementById('add_user_button').disabled=true;
	}else
	{
		document.getElementById('add_user_button').disabled=false;
	}	
	for(var i = 1; i < user_count; i++)
	{
		var tr = tbl.insertRow(tbl.rows.length);
		var td;
		var str;

		td = tr.insertCell(tr.cells.length);
		td.innerHTML = eval("dvr_data.user.user_list.user" + i + ".name");
		
		td = tr.insertCell(tr.cells.length);
		var permit_admin = "";
//		var permit_live = "";
		var permit_setting = "";
		var permit_playback = "";
		
		if(eval("dvr_data.user.user_list.user" + i + ".admin") == "yes")
		{
			permit_admin = "checked";
		}
//		if(eval("dvr_data.user.user_list.user" + i + ".permit_live") == "yes")
//		{
//			permit_live = "checked";
//		}
		if(eval("dvr_data.user.user_list.user" + i + ".permit_setting") == "yes")
		{
			permit_setting = "checked";
		}
		if(eval("dvr_data.user.user_list.user" + i + ".permit_playback") == "yes")
		{
			permit_playback = "checked";
		}
		str = "";
                    str += "<input type=\"checkbox\" id=\"permit_admin\" " + permit_admin + "/>permit_admin";
            //		str += "<input type=\"checkbox\" id=\"permit_live\" " + permit_live + ">permit_live";
                    str += "<input type=\"checkbox\" id=\"permit_setting\" " + permit_setting + "/>permit_setting";
                    str += "<input type=\"checkbox\" id=\"permit_playback\" " + permit_playback + "/>playback";
                    td.innerHTML = str;
		
		td = tr.insertCell(tr.cells.length);
		var edit_user = "";
		var del_user = "";
//		var set_pass = "";
		edit_user = "disabled"
		if(eval("dvr_data.user.user_list.user" + i + ".edit_user") == "no")
		{
			edit_user = "disabled";
		}
		if(eval("dvr_data.user.user_list.user" + i + ".del_user") == "no")
		{
			del_user = "disabled";
		}
//		if(eval("dvr_data.user.user_list.user" + i + ".set_pass") == "no")
//		{
//			set_pass = "disabled";
//		}
		str = "";
		str += "<button type=\"button\" id=\"edit_user\" " + edit_user + " onclick=\"user_management_target='" + eval("dvr_data.user.user_list.user" + i + ".name") + "';user_management_save_edit_usr()\">save</button>";
		str += "<button type=\"button\" id=\"del_user\" " + del_user + " onclick=\"user_management_target='" + eval("dvr_data.user.user_list.user" + i + ".name") + "';user_management_prepare_rm()\">del_user</button>";
//		str += "<input type=\"button\" id=\"set_pass\" value=\"设置\" " + set_pass + ">";
		td.innerHTML = str;
	}
}

function user_management_load_content()
{
	dvr_ajax = $.ajax({ 
		type:"GET",
		url: ipc_url + "/user/user_list.xml", 
		processData: false, 
		cache: false,
		data: "username=" + g_usr + "&password=" + g_pwd, 
		async:true,
//		dataType: 'get',
//		jsonp: 'jsoncallback',
		beforeSend: function(XMLHttpRequest){
		},
		success: function(data, textStatus, xmlhttp){
//			alert("recv:" + data);
			var dvr_data = xml2json.parser(xmlhttp.responseText, "", false);
			if(dvr_data.user.ret != "success")
			{
				alert('login_fail');	
			}
			else
			{
				user_management_data2ui(dvr_data);
			}
		},
		complete: function(XMLHttpRequest, textStatus){
//			alert("complete:" + textStatus);
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
			alert("error:" + textStatus);
		}
	});	
}
//time
var strYear,strMonth,strDate,strHour,strMin,strSen;
Date.prototype.toFormatString=function()
{
	strYear=this.getUTCFullYear().toString();
	if (strYear.length<4)
	{
		var i = 4-strYear.length;
		for (var j = 0; j < i; j++)
		{
			strYear = "0" + strYear;
		}
	}
	strMonth=(this.getUTCMonth()+parseInt(1)).toString();
	strMonth=(strMonth.length==1)?("0"+strMonth):strMonth
	strDate=this.getUTCDate().toString();
	strDate=(strDate.length==1)?("0"+strDate):strDate
	strHour=this.getUTCHours().toString();
	strHour=(strHour.length==1)?("0"+strHour):strHour
	strMin=this.getUTCMinutes().toString();
	strMin=(strMin.length==1)?("0"+strMin):strMin
	strSen=this.getUTCSeconds().toString();
	strSen=(strSen.length==1)?("0"+strSen):strSen
}
function time_zone_load()
{
	//var auth = "Basic " + base64.encode(g_usr+':'+g_pwd);
	var auth = "Basic " + base64.encode('admin:');
		$.ajax({
				type:"GET",
				url:ipc_url + '/System/time/timeZone/',
				dataType:"json",
				beforeSend : function(req){ 
				req .setRequestHeader('Authorization', auth);
				},
				success:function(data){	
					$('#time_zone').innerHTML = data;
				},
				error:function(a,b,c){ 
					alert(b);
			}
		});
}
function time_zone_save()
{
	var time_zone = $('#time_zone :selected').html();
	//var auth = "Basic " + base64.encode(g_usr+':'+g_pwd);
	var auth = "Basic " + base64.encode('admin:');
	
	var time_zone_data = '{"time_Zone":"'+time_zone+'"}';
		$.ajax({
				type:'PUT',
				url:ipc_url + '/System/time/timeZone/',
				dataType:'json',
				data:time_zone_data,
				async:false,
				beforeSend : function(req ) {
				req .setRequestHeader('Authorization', auth);
				},
				success:function(data){ 
					if(data.statusCode == 0){ 

					}
				},
				error:function(a,b,c){ 
					alert(b);
				}
			})
}
function time_data2ui(dvr_data)
{
	var utc_devtime = parseInt(dvr_data.juan.setup.time.value)*1000;
	$("#time_ntp__server")[0].innerHTML = dvr_data.juan.conf.datetime.ntp_user_domain;
	
	var devtime = new Date(utc_devtime);
	devtime.toFormatString();
	switch(dvr_data.juan.conf.datetime.date_separator)
	{
		case "-": $("#date_break")[0].innerHTML = 'xxxx-xx-xx';break;
		case "/": $("#date_break")[0].innerHTML = 'xxxx/xx/xx';break;
		case ".": $("#date_break")[0].innerHTML = 'xxxx.xx.xx';break;
		default: break;
	}
	switch(dvr_data.juan.conf.datetime.date_format)
	{
		case "yyyymmdd": 
			$(".date_form")[0].innerHTML = 'YMD: xxxx xx xx';
			break;
		case "mmddyyyy": 
			$(".date_form")[0].innerHTML = 'MDY: xx xx xxxx';
			break;
		case "ddmmyyyy": 
			$(".date_form")[0].innerHTML = 'DMY: xx xx xxxx';
			break;
		default: break;
	}
	$("#daylight_time")[0].value = dvr_data.juan.conf.datetime.day_saving_time;

	switch (dvr_data.juan.conf.datetime.ntp_sync)
	{
		case "yes": $("#time_ntp__1")[0].checked = 1;break;
		case "no": $("#time_ntp__0")[0].checked = 1;break;
		default:break;	
	};
	ntp_change();

	showtime();
}
function time_load_content()
{
	var xmlstr = '';
	xmlstr += '<juan ver="" seq="">';
	xmlstr += '<conf type="read" user="admin" password="">';
	xmlstr += '<datetime date_format="" date_separator="" time_format="" day_saving_time="" ntp_sync="" ntp_user_domain=""	/>';
	xmlstr += '</conf>';
	xmlstr += '<setup type="read" user="admin" password="">';
	xmlstr += '<time value="" />';
	xmlstr += '</setup>';
	xmlstr += '</juan>';
	//alert(xmlstr);

	dvr_ajax = $.ajax({ 
		type:"GET",
		url: ipc_url + "/cgi-bin/gw2.cgi?f=j", 
		processData: false, 
		cache: false,
		data: "xml=" + xmlstr, 
		async:true,
		dataType: 'jsonp',
		jsonp: 'jsoncallback',

		beforeSend: function(XMLHttpRequest){
		},
		success: function(data, textStatus){
//			alert("recv:" + data.xml);
			var dvr_data = xml2json.parser(data.xml, "", false);
			//time_zone_load();
			time_data2ui(dvr_data);
			
		},
		complete: function(XMLHttpRequest, textStatus){
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
			//alert(textStatus);
		}
	});	
}

function time_save_content()
{
	var date_form,date_break;
	switch ($(".date_form")[0].innerHTML)
	{
		case 'YMD: xxxx xx xx':
			date_form = "yyyymmdd";break;
		case 'MDY: xx xx xxxx':
			date_form = "mmddyyyy";break;
		case 'DMY: xx xx xxxx':
			date_form = "ddmmyyyy";break;
		default:
			break;
	}
	switch ($("#date_break")[0].innerHTML)
	{
		case 'xxxx-xx-xx':
			date_break = "-";break;
		case 'xxxx/xx/xx':
			date_break = "/";break;
		case 'xxxx.xx.xx':
			date_break = ".";break;
		default:
			break;		
	}
	var ntp_s,ntp_domain;
	if ($("#time_ntp__1")[0].checked == true)
	{
		ntp_s = "yes";
	}else ntp_s = "no";
	ntp_domain = $("#time_ntp__server")[0].innerHTML;
	savetime();

	var xmlstr = '';
	xmlstr += '<juan ver="1.0" seq="0">';
	xmlstr += '<conf type="write" user="admin" password="">';
	xmlstr += '<datetime date_format="' + date_form + '" date_separator="' + date_break + '" time_zone="' + $("#time_zone")[0].innerHTML + '" day_saving_time="' + $("#daylight_time")[0].value + '" ntp_sync="' + ntp_s + '" ntp_user_domain="' + ntp_domain + '"	/>';
	xmlstr += '</conf>';
	xmlstr += '</juan>';
//	alert(xmlstr);

	dvr_ajax = $.ajax({ 
		type:"GET",
		url: ipc_url + "/cgi-bin/gw2.cgi?f=j", 
		processData: false, 
		cache: false,
		data: "xml=" + xmlstr, 
		async:true,
		dataType: 'jsonp',
		jsonp: 'jsoncallback',

		beforeSend: function(XMLHttpRequest){
		},
		success: function(data, textStatus){
//			alert("recv:" + data.xml);
		},
		complete: function(XMLHttpRequest, textStatus){
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
		}
	});	
}
function showtime()
{
	var time_show = "";
	switch ($("#date_break")[0].innerHTML)
	{
		case 'xxxx-xx-xx':
			datebreak = '-';
			break;
		case 'xxxx/xx/xx':
			datebreak = '/';
			break;
		case 'xxxx.xx.xx':
			datebreak = '.';
			break;
		default:
			break;		
	}
	switch ($(".date_form")[0].innerHTML)
	{
		case 'YMD: xxxx xx xx':
			time_show = yy+datebreak+mm+datebreak+dd+"  ";
			break;
		case 'MDY: xx xx xxxx':
			time_show = mm+datebreak+dd+datebreak+yy+"  ";
			break;
		case 'DMY: xx xx xxxx':
			time_show = dd+datebreak+mm+datebreak+yy+"  ";
			break;
		default:
			break;
	}
	time_show += strHour+":"+strMin+":"+strSen;
	$("#curent_time")[0].value = time_show;
}
function savetime()
{
	var currentset_date = new Date();
	currentset_date.setFullYear(parseInt(strYear, 10));
	currentset_date.setMonth(parseInt(strMonth, 10)-1);
	currentset_date.setDate(parseInt(strDate, 10));
	currentset_date.setHours(parseInt(strHour, 10));
	currentset_date.setMinutes(parseInt(strMin, 10));
	currentset_date.setSeconds(parseInt(strSen, 10));
	var currentset_utc = currentset_date.getTime()/1000;
// + currentset_date.getTimezoneOffset()*60;
//	alert(currentset_utc);

	var xmlstr = '';
	xmlstr += '<juan ver="1.0" seq="0">';
	xmlstr += '<setup type="write" user="admin" password="">';
	xmlstr += '<time value="' + currentset_utc + '" />';
	xmlstr += '</setup>';
	xmlstr += '</juan>';
//	alert(xmlstr);

	dvr_ajax = $.ajax({ 
		type:"GET",
		url: ipc_url + "/cgi-bin/gw2.cgi?f=j", 
		processData: false, 
		cache: false,
		data: "xml=" + xmlstr, 
		async:true,
		dataType: 'jsonp',
		jsonp: 'jsoncallback',

		beforeSend: function(XMLHttpRequest){
		},
		success: function(data, textStatus){
		},
		complete: function(XMLHttpRequest, textStatus){
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
		}
	});	
}
function sync_pc_time()
{
	strYear = yy;
	strMonth = mm;
	strDate = dd;
	strHour = hh;
	strMin = mi;
	strSen = ss;
	showtime();
	savetime();
}
function manual_set_time()
{
	var date_sep,time_sep;
	date_sep = $(".m_date")[0].value.split("-");
	time_sep = $('.in');//$("#m_time")[0].value.split(":");
	strYear = date_sep[0]; strHour = $('.in').eq(0).val();
	strMonth = date_sep[1]; strMin = $('.in').eq(1).val();
	strDate = date_sep[2]; strSen = $('.in').eq(2).val();
	showtime();
	savetime();
}

function is_valid_zone()
{
	var obj_zone=document.getElementById('time_zone');
	var str=obj_zone.value;
	var a = str.match(/^-?[1-9]$|^-?1[1-2]$|^0$/);
	if (a == null) 
	{
		alert('时区范围：\n     [-12~12]');
		obj_zone.value="8";
		return false;
	}
	return true;
}

function is_valid_daylight()
{
	var obj_daylight=document.getElementById('daylight_time');
	var str=obj_daylight.value;
	var a = str.match(/^-?[1-3]$|^0$/);
	if (a == null) 
	{
		alert('夏令时范围：\n     [-3~3]');
		obj_daylight.value="0";
		return false;
	}
	return true;
}

function is_valid_time()
{
	obj_time=document.getElementById('m_time');
	var str=obj_time.value;
	var a = str.match(/^(\d{2})(:)?(\d{2})\2(\d{2})$/);
	if (a == null) 
	{
		alert('format_wrong'+'13:23:05');
		obj_time.value="00:00:00";
		return false;
	}
	if (a[1]>24 || a[3]>60 || a[4]>60)
	{
		alert('format_wrong'+'13:23:05');
		obj_time.value="00:00:00";
		return false;
	}
	return true;
}

function is_valid_date()
{
	obj_date=document.getElementByClassName('m_date');
	var str=obj_date.value;
	var r = str.match(/^(\d{1,4})(-|\/)(\d{2})\2(\d{2})$/);
	if(r==null)
	{
		alert('format_wrong'+"2012-01-01");
		obj_date.value="0000-00-00";
		return false;
	}
	var d= new Date(r[1], r[3]-1, r[4]);
	if ((d.getFullYear()==r[1]&&(d.getMonth()+1)==r[3]&&d.getDate()==r[4])==false)
	{
		alert('format_wrong'+"2012-01-01");
		obj_date.value="0000-00-00";
		return false;
	}
	return true;
}
function ntp_change()
{
	if ($("#time_ntp__1")[0].checked == 1)
	{
		$("#time_ntp__server")[0].disabled = false;
	}else
	{
		$("#time_ntp__server")[0].disabled = true;
	}
}
//reboot
function reboot()
{
	var xmlstr = '';
	xmlstr += '<juan ver="1.0" seq="0">';
	xmlstr += '<setup type="write" user="admin" password="">';
	xmlstr += '<system operation="reboot" />';
	xmlstr += '</setup>';
	xmlstr += '</juan>';
//	alert(xmlstr);

	dvr_ajax = $.ajax({ 
		type:"GET",
		url: ipc_url + "/cgi-bin/gw2.cgi?f=j", 
		processData: false, 
		cache: false,
		data: "xml=" + xmlstr, 
		async:true,
//		dataType: 'jsonp',
//		jsonp: 'jsoncallback',


		beforeSend: function(XMLHttpRequest){
		},
		success: function(data, textStatus){
//			alert("recv:" + data.xml);
			var dvr_data = xml2json.parser(data, "", false);
	
		},
		complete: function(XMLHttpRequest, textStatus){
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
			//alert(textStatus);
		}
	});	
}
//default_setting
function default_setting()
{
	var xmlstr = '';
	xmlstr += '<juan ver="1.0" seq="0">';
	xmlstr += '<setup type="write" user="admin" password="">';
	xmlstr += '<system operation="default factory" />';
	xmlstr += '</setup>';
	xmlstr += '</juan>';
//	alert(xmlstr);

	dvr_ajax = $.ajax({ 
		type:"GET",
		url: ipc_url + "/cgi-bin/gw2.cgi?f=j", 
		processData: false, 
		cache: false,
		data: "xml=" + xmlstr, 
		async:true,
//		dataType: 'jsonp',
//		jsonp: 'jsoncallback',


		beforeSend: function(XMLHttpRequest){
		},
		success: function(data, textStatus){
//			alert("recv:" + data.xml);
			var dvr_data = xml2json.parser(data, "", false);
		},
		complete: function(XMLHttpRequest, textStatus){
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
			//	alert(textStatus);
		}
	});	
}
//upLoad
var upload_persent = 0;
$(function(){
	$('#swfupload-control').swfupload({
		upload_url: ipc_url + "/cgi-bin/upload.cgi",
		file_size_limit : "16384",
		file_types : "*.rom",
		file_types_description : "Upgrade File",
		file_upload_limit : "0",
		flash_url : ipc_url + "/images/swfupload.swf",
		button_image_url :ipc_url + 'images/XPButtonUploadText_61x221.png',
		button_width : 98,
		button_height : 22,
//		button_text_top_padding : 5,
		button_text_left_padding : 35,
		button_text_style : "font-size: 22px;",
		button_text : 'up',
		button_disabled : false,
		button_placeholder : $('#button_upload')[0],
		debug: false,
		custom_settings : {something : "here"}
	})
	.bind('swfuploadLoaded', function(event){
		//$('#log').append('<li>Loaded</li>');
		$('#txt_status')[0].innerHTML = "";
	})
	.bind('fileQueued', function(event, file){
		//$('#log').append('<li>File queued - '+file.name+'</li>');
		// start the upload since it's queued
		$(this).swfupload('startUpload');
	})
	.bind('fileQueueError', function(event, file, errorCode, message){
		//$('#log').append('<li>File queue error - '+message+'</li>');
		$('#txt_status')[0].innerHTML = 'error'+ message;
		$('#txt_progress')[0].innerHTML = "";
	})
	.bind('fileDialogStart', function(event){
		//$('#log').append('<li>File dialog start</li>');
	})
	.bind('fileDialogComplete', function(event, numFilesSelected, numFilesQueued){
		//$('#log').append('<li>File dialog complete</li>');
	})
	.bind('uploadStart', function(event, file){
		//$('#log').append('<li>Upload start - '+file.name+'</li>');
		$('#txt_status')[0].innerHTML = 'start_upload';
		$(this).swfupload('setButtonDisabled', true);
	})
	.bind('uploadProgress', function(event, file, bytesLoaded){
		//$('#log').append('<li>Upload progress - '+bytesLoaded+'</li>');
		var str = "";
		var persent = bytesLoaded/file.size*100 + "";
		upload_persent = bytesLoaded/file.size*100;
		for(var i = 0; i < bytesLoaded/file.size*10; i++){
			str += "|";
		}
		if(upload_persent < 100){
			persent = persent.substr(0, 2);
		}
		if(upload_persent >= 100){
			persent = persent.substr(0, 3);
		}
		str += persent + "%";
		$('#txt_progress')[0].innerHTML = str;
	})
	.bind('uploadSuccess', function(event, file, serverData){
		//$('#log').append('<li>Upload success - '+file.name+'</li>');
		$('#txt_status')[0].innerHTML = 'stop_upload';
	})
	.bind('uploadComplete', function(event, file){
		//$('#log').append('<li>Upload complete - '+file.name+'</li>');
		if(upload_persent >= 100)
		{
			$('#txt_status')[0].innerHTML = 'stop_upload';
			get_upgrade_rate();
		}
		else
		{
			$('#txt_status')[0].innerHTML = 'wait_reboot';
		}
		// upload has completed, lets try the next one in the queue
		$(this).swfupload('startUpload');
	})
	.bind('uploadError', function(event, file, errorCode, message){
		//$('#log').append('<li>Upload error - '+message+'</li>');
		$('#txt_status')[0].innerHTML = 'fail_upload';
	});
});	

var upgrade_persent = 0;
function get_upgrade_rate()
{
	$.ajax({ 
		type:"GET",
		url: ipc_url+ "/cgi-bin/upgrade_rate.cgi?cmd=upgrade_rate", 
		processData: false, 
		cache: false,
		data: "", 
		async:true,
		beforeSend: function(XMLHttpRequest){
			//alert("beforeSend");
		},
		success: function(data, textStatus){
//			alert("recv:" + data);
			$('#txt_upgrade_status')[0].innerHTML = 'writing_firmware';
			upgrade_persent = parseInt(data);
			
			var str = "";
			var persent = upgrade_persent + "";
			for(var i = 0; i < persent/10; i++){
				str += "|";
			}
			if(persent != "100"){
				persent = persent.substr(0, 2);
			}
			str += persent + "%";
			$('#txt_upgrade_progress')[0].innerHTML = str;
		},
		complete: function(XMLHttpRequest, textStatus){
			//alert("complete:" + textStatus);
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
			//alert("error");	
		}
	});
	
	if(upgrade_persent <= 99)
	{
		setTimeout("get_upgrade_rate()", 1000);
	}
	else
	{
		alert('upgrade_success');
	}
}
