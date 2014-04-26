var dvr_url,dvr_usr,dvr_pwd,dvr_chn;
var dvr_data = null;
var dvr_selected_chn = 0;
var dvr_selected_weekday = 0;
var dvr_ajax;

function dvr(_url,_usr,_pwd,_chn){
dvr_url = _url;
dvr_usr = _usr;
dvr_pwd = _pwd;
dvr_chn = _chn;
}
//*****************************           dvr's  common code. (Don't Compile Here!!!!)         **********************
function mk_chn_content(_id, _chn)
{
	//alert(_chn)
	$('#'+_id).find('li').remove();
	for(var i = 0; i < _chn; i++)
	{
		var num = i+1;
		$('#'+_id).append('<li class="add_li_1"><a href="javascript:;">'+num+'</a></li>');
	}
	for(var ii = 0; ii < 32; ii++)
	{
		$('.'+_id+' div input').eq(ii).prop('checked',false);
		if(ii >= _chn)
		{
			$('.'+_id+' div input').eq(ii).prop('disabled',true);
		}
	}
	$('.'+_id+' div input').eq(0).prop('checked',true);
	$('.'+_id+' div input').eq(0).prop('disabled',true);
		$('.chk_all').each(function(index) {
			$(this).click(function(){
				for(var i = 0; i < _chn; i++)
				{
					if($(this).prop('checked') == true){
						$('.'+_id+' div input').eq(i).prop('checked',true);
					}else{
						$('.'+_id+' div input').eq(i).prop('checked',false);
					}
				}
				var dvr_selected_chn = $('#'+_id+'0')[0].innerHTML - 1;
				$('.'+_id+' div input').eq(dvr_selected_chn).prop('checked',true);
				$('.'+_id+' div input').eq(dvr_selected_chn).prop('disabled',true);
			})
		});
	$('.chk_all_btn').each(function(index) {
			$(this).click(function(){
				$('.chk_all').eq(index).prop('checked',true);
				for(var i = 0; i < _chn; i++)
				{
					$('.'+_id+' div input').eq(i).prop('checked',true);
				}
				var dvr_selected_chn = $('#'+_id+'0')[0].innerHTML - 1;
				$('.'+_id+' div input').eq(dvr_selected_chn).prop('checked',true);
				$('.'+_id+' div input').eq(dvr_selected_chn).prop('disabled',true);
			})
		});
	$('#'+_id+' li a').each(function(index) {
				$(this).click(function(){
					$('#'+_id+'0').html($(this)[0].innerHTML);
					for(var i = 0; i < _chn; i++)
					{
						$('.chk_all').eq(i).prop('checked',false);
						$('.'+_id+' div input').eq(i).prop('checked',false);
						$('.'+_id+' div input').eq(i).prop('disabled',false);
					}
					$('.'+_id+' div input').eq(index).prop('checked',true);
					$('.'+_id+' div input').eq(index).prop('disabled',true);
					dvr_chn_change(_id);
			})
		});
}
function dvr_save(s_id)
{
	var ret = 0;
	for(var chk_chn = 0; chk_chn < dvr_chn; chk_chn++)
	{
		if($('.'+s_id+' div input').eq(chk_chn).prop('checked') == true){
			ret += Math.pow(2, chk_chn);
		}
	}
	switch(s_id)
	{
		case 'dvr_record_chn_sel':dvr_record_save_content(ret);break;
		case 'dvr_enc_chn_sel':dvr_encoding_save_content(ret);break;
		case 'dvr_ptz_chn_sel':dvr_ptz_save_content(ret);break;
		case 'dvr_detect_chn_sel':dvr_detect_save_content(ret);break;
		case 'dvr_alarm_chn_sel':dvr_alarm_save_content(ret);break;
		default: break;
	}
}
function dvr_load(id_chn)
{
	mk_chn_content(id_chn, dvr_chn);
	switch(id_chn)
	{
		case 'dvr_screen_chn_sel':dvr_screen_load_content();break;
		case 'dvr_record_chn_sel':dvr_record_load_content(dvr_selected_chn,dvr_selected_weekday);break;
		case 'dvr_enc_chn_sel':dvr_encoding_load_content();break;
		case 'dvr_ptz_chn_sel':dvr_ptz_load_content();break;
		case 'dvr_detect_chn_sel':dvr_detect_load_content();break;
		case 'dvr_alarm_chn_sel':dvr_alarm_load_content();break;
		default: break;
	}
}
function dvr_chn_change(_id)
{
	var dvr_selected_chn = $("#"+_id+'0')[0].innerHTML - 1;
	switch(_id)
	{
		case 'dvr_screen_chn_sel':dvr_screen_data2ui(dvr_selected_chn);break;
		case 'dvr_record_chn_sel':dvr_record_load_content(dvr_selected_chn);break;
		case 'dvr_enc_chn_sel':dvr_encoding_data2ui(dvr_selected_chn);break;
		case 'dvr_ptz_chn_sel':dvr_ptz_data2ui(dvr_selected_chn);break;
		case 'dvr_detect_chn_sel':dvr_detect_data2ui(dvr_selected_chn);break;
		case 'dvr_alarm_chn_sel':dvr_alarm_data2ui(dvr_selected_chn);break;
		default: break;
	}
}
//*************************************************************************************************************************************************



//devinfo
function dvr_devinfo_load_content()
{
	var xmlstr = '';
	xmlstr += '<juan ver="0" squ="fastweb" dir="0">';
	xmlstr += '<envload type="0" usr="' + dvr_usr + '" pwd="' + dvr_pwd + '"/>';
	xmlstr += '<devinfo name="" model="" hwver="" swver="" reldatetime="" ip="" httpport="" clientport="" rip="" rhttpport="" rclientport="" camcnt="" audcnt="" sensorcnt="" alarmcnt="" />';
	xmlstr += '</juan>';
	//alert(xmlstr);

	dvr_ajax = $.ajax({ 
		type:"GET",
		url: dvr_url + "/cgi-bin/gw.cgi?f=j", 
		processData: false, 
		cache: false,
		data: "xml=" + xmlstr, 
		async:true,
		dataType: 'jsonp',
		jsonp: 'jsoncallback',
		beforeSend: function(XMLHttpRequest){
//			alert("beforeSend");
		},
		success: function(data, textStatus){
//			alert("recv:" + data.xml);
			dvr_data = xml2json.parser(data.xml, "", false)
			if(dvr_data.juan.envload.errno != 0)
			{
				alert_error(dvr_data.juan.envload.errno);
				return;
			}
			$("#Dvr_Device_name")[0].value = dvr_data.juan.devinfo.name;
			$("#Dvr_Device_model")[0].value = dvr_data.juan.devinfo.model;
			$("#Dvr_Hardware_version")[0].value = dvr_data.juan.devinfo.hwver;
			$("#Dvr_Software_version")[0].value = dvr_data.juan.devinfo.swver;//
			$("#Dvr_Build_time")[0].value = dvr_data.juan.devinfo.reldatetime;	
			$('#Dvr_Sum_of_channels')[0].value = dvr_data.juan.devinfo.camcnt;
		},
		complete: function(XMLHttpRequest, textStatus){
//			alert("complete:" + textStatus);
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
			//alert(textStatus);
		}
	});	
}
//commonsettings
function dvr_common_data2ui()
{
	switch(dvr_data.juan.envload.misc.datefmt)
	{
		case 0:	$("#dvr_common_date_format")[0].innerHTML = 'YYYY/MM/DD';break;
		case 1: $("#dvr_common_date_format")[0].innerHTML = 'MM/DD/YYYY';break;
		case 2: $("#dvr_common_date_format")[0].innerHTML = 'DD/MM/YYYY';break;
		default: break;
	}
	if(dvr_data.juan.envload.misc.keylock == 1){
		$('#dvr_common_keypad').prop('checked',true);
	}else{
		$('#dvr_common_keypad').prop('checked',false);
	}
	if(dvr_data.juan.envload.misc.keybuzzer == 1){
		$('#dvr_common_key_sound').prop('checked',true);
	}else{
		$('#dvr_common_key_sound').prop('checked',false);
	}
	switch(dvr_data.juan.envload.misc.standard)
	{
		case 0:	$("#dvr_common_standard")[0].innerHTML = 'PAL';break;
		case 1: $("#dvr_common_standard")[0].innerHTML = 'NTST';break;
		default: break;
	}
	$("#dvr_common_dvrid")[0].value = dvr_data.juan.envload.misc.dvrid;
	if(dvr_data.juan.envload.misc.hddoverwrite == 1){
		$('#dvr_common_over_harddisk').prop('checked',true);
	}else{
		$('#dvr_common_over_harddisk').prop('checked',false);
	}
	$("#dvr_common_OSD_alpha")[0].innerHTML = dvr_data.juan.envload.misc.alpha;
	if(dvr_data.juan.envload.misc.autoswi == 1){
		$('#dvr_common_autoswi').prop('checked',true);
	}else{
		$('#dvr_common_autoswi').prop('checked',false);
	}
	switch(dvr_data.juan.envload.misc.autoswiinterval)
	{
		case 0:	$("#dvr_common_autoswiinterval")[0].innerHTML = '2s';break;
		case 1: $("#dvr_common_autoswiinterval")[0].innerHTML = '3s';break;
		case 2:	$("#dvr_common_autoswiinterval")[0].innerHTML = '4s';break;
		case 3: $("#dvr_common_autoswiinterval")[0].innerHTML = '5s';break;
		case 4:	$("#dvr_common_autoswiinterval")[0].innerHTML = '8s';break;
		case 5: $("#dvr_common_autoswiinterval")[0].innerHTML = '10s';break;
		default: break;
	}
	$('#dvr_common_autoswimode_ID').val(dvr_data.juan.envload.misc.autoswimode);
	switch(dvr_data.juan.envload.misc.autoswimode)
	{
		case 0:	$("#dvr_common_autoswimode")[0].innerHTML = lang.Full_screen;break;
		case 1: $("#dvr_common_autoswimode")[0].innerHTML = lang.Split_screen;break;
		default: break;
	}
	$('#dvr_common_autoswimode_ID').nextAll('li').click(function(){
		 	$('#dvr_common_autoswimode_ID').val($(this).attr('value'));
		})
}
function dvr_common_load_content()
{
	var xmlstr = '';
	xmlstr += '<juan ver="0" squ="fastweb" dir="0">';
	xmlstr += '<envload type="0" usr="' + dvr_usr + '" pwd="' + dvr_pwd + '">';
	xmlstr += '<misc datefmt="" keylock="" keybuzzer="" lang="" standard="" dvrid="" hddoverwrite="" alpha="" autoswi="" autoswiinterval="" autoswimode="" />';
	xmlstr += '</envload>';
	xmlstr += '</juan>';
//	alert(xmlstr);

	dvr_ajax = $.ajax({ 
		type:"GET",
		url: dvr_url + "/cgi-bin/gw.cgi?f=j", 
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
			dvr_data = xml2json.parser(data.xml, "", false)
			if(dvr_data.juan.envload.errno != 0)
			{
				alert_error(dvr_data.juan.envload.errno);
				return;
			}
			dvr_common_data2ui();
		},
		complete: function(XMLHttpRequest, textStatus){
//			alert("complete:" + textStatus);
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
			//alert(textStatus);
		}
	});	
}

function dvr_common_save_content()
{
	var dvr_common_date_format,dvr_common_standard,dvr_common_autoswiinterval,dvr_common_autoswimode;
	switch($("#dvr_common_date_format")[0].innerHTML)
	{
		case 'YYYY/MM/DD': dvr_common_date_format = 0;break;
		case 'MM/DD/YYYY': dvr_common_date_format = 1;break;
		case 'DD/MM/YYYY': dvr_common_date_format = 2;break;
		default: break;
	}
	var dvr_common_keypad = $('#dvr_common_keypad').prop('checked') ?1:0;
	var dvr_common_key_sound = $('#dvr_common_key_sound').prop('checked') ?1:0;
	switch($("#dvr_common_standard")[0].innerHTML)
	{
		case 'PAL':	dvr_common_standard = 0;break;
		case 'NTST': dvr_common_standard = 1;break;
		default: break;
	}
	var dvr_common_over_harddisk = $('#dvr_common_over_harddisk').prop('checked') ?1:0;
	var dvr_common_autoswi = $('#dvr_common_autoswi').prop('checked') ?1:0;
	switch($("#dvr_common_autoswiinterval")[0].innerHTML)
	{
		case '2s': dvr_common_autoswiinterval = 0;break;
		case '3s': dvr_common_autoswiinterval = 1;break;
		case '4s': dvr_common_autoswiinterval = 2;break;
		case '5s': dvr_common_autoswiinterval = 3;break;
		case '8s': dvr_common_autoswiinterval = 4;break;
		case '10s': dvr_common_autoswiinterval = 5;break;
		default: break;
	}
	var xmlstr = '';
	xmlstr += '<juan ver="0" squ="fastweb" dir="0">';
	xmlstr += '<envload type="1" usr="' + dvr_usr + '" pwd="' + dvr_pwd + '">';
	xmlstr += '<misc';
	xmlstr += ' datefmt="' + dvr_common_date_format + '"';
	xmlstr += ' keylock="' + dvr_common_keypad + '"';
	xmlstr += ' keybuzzer="' + dvr_common_key_sound + '"';
	xmlstr += ' standard="' + dvr_common_standard + '"';
	xmlstr += ' dvrid="' + $("#dvr_common_dvrid")[0].value + '"';
	xmlstr += ' hddoverwrite="' + dvr_common_over_harddisk + '"';
	xmlstr += ' alpha="' + $("#dvr_common_OSD_alpha")[0].innerHTML + '"';
	xmlstr += ' autoswi="' + dvr_common_autoswi + '"';
	xmlstr += ' autoswiinterval="' + dvr_common_autoswiinterval + '"';
	xmlstr += ' autoswimode="' + $("#dvr_common_autoswimode_ID")[0].value + '"';
	xmlstr += ' />';
	xmlstr += '</envload>';
	xmlstr += '</juan>';
//	alert(xmlstr);

	dvr_ajax = $.ajax({ 
		type:"GET",
		url: dvr_url + "/cgi-bin/gw.cgi?f=j", 
		processData: false, 
		cache: false,
		data: "xml=" + xmlstr, 
		async:true,
		dataType: 'jsonp',
		jsonp: 'jsoncallback',

		beforeSend: function(XMLHttpRequest){
//			alert("beforeSend");
		},
		success: function(data, textStatus){
//			alert("recv:" + data.xml);
			var ret_data = xml2json.parser(data.xml, "", false)

			if(ret_data.juan.envload.errno != 0)
			{
				alert_error(ret_data.juan.envload.errno);
				return;
			}
		},
		complete: function(XMLHttpRequest, textStatus){
//			alert("complete:" + textStatus);
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
			//alert(textStatus);
		}
	});	
}
//network
function dvr_network_data2ui()
{
	if(dvr_data.juan.envload.network.dhcp == 1){
		$('#dvr_openDH').prop('checked',true);
	}else{
		$('#dvr_openDH').prop('checked',false);
	}
	if(dvr_data.juan.envload.network.ddns == 1){
		$('#dvr_openDD').prop('checked',true);
	}else{
		$('#dvr_openDD').prop('checked',false);
	}
	if(dvr_data.juan.envload.network.pppoe == 1){
		$('#dvr_Open_PPPOE').prop('checked',true);
	}else{
		$('#dvr_Open_PPPOE').prop('checked',false);
	}
	$("#dvr_mac_addr")[0].value = dvr_data.juan.envload.network.mac;
	switch(dvr_data.juan.envload.network.ddnsprovider)
	{
		case 0: $("#dvr_supplier")[0].value = 'dyndns.org';break;
		case 1: $("#dvr_supplier")[0].value = 'no-ip.com';break;
		case 2: $("#dvr_supplier")[0].value = 'changeip.com';break;
		case 3: $("#dvr_supplier")[0].value = '3322.org';break;
		case 4: $("#dvr_supplier")[0].value = 'popdvr.com';break;
		default: break;
	}
	$("#dvr_ip_addr")[0].value = dvr_data.juan.envload.network.ip;
	$("#dvr_subnet_mask")[0].value = dvr_data.juan.envload.network.submask;
	$("#dvr_network_addr")[0].value = dvr_data.juan.envload.network.gateway;
	$("#dvr_dns_addr")[0].value = dvr_data.juan.envload.network.dns;
	$("#dvr_web_port")[0].value = dvr_data.juan.envload.network.httpport;
	$("#dvr_client_port")[0].value = dvr_data.juan.envload.network.clientport;
	$("#dvr_eseeID")[0].value = dvr_data.juan.envload.network.enetid;
	$("#dvr_domain")[0].value = dvr_data.juan.envload.network.ddnsurl;
	$("#dvr_user_DD")[0].value = dvr_data.juan.envload.network.ddnsusr;
	$("#dvr_password_DD")[0].value = dvr_data.juan.envload.network.ddnspwd;
	$("#dvr_user_PPPOE")[0].value = dvr_data.juan.envload.network.pppoeusr;
	$("#dvr_password_PPPOE")[0].value = dvr_data.juan.envload.network.pppoepwd;
}
function dvr_network_load_content()
{
	var xmlstr = '';
	xmlstr += '<juan ver="0" squ="fastweb" dir="0">';
	xmlstr += '<envload type="0" usr="' + dvr_usr + '" pwd="' + dvr_pwd + '">';
	xmlstr += '<network dhcp="" mac="" ip="" submask="" gateway="" dns="" httpport="" clientport="" enetid="" ddns="" ddnsprovider="" ddnsurl="" ddnsusr="" ddnspwd="" pppoe="" pppoeusr="" pppoepwd="" />';
	xmlstr += '</envload>';
	xmlstr += '</juan>';
	//alert(xmlstr);

	dvr_ajax = $.ajax({ 
		type:"GET",
		url: dvr_url + "/cgi-bin/gw.cgi?f=j", 
		processData: false, 
		cache: false,
		data: "xml=" + xmlstr, 
		async:true,
		dataType: 'jsonp',
		jsonp: 'jsoncallback',

		beforeSend: function(XMLHttpRequest){
//			alert("beforeSend");
		},
		success: function(data, textStatus){
//		alert("recv:" + data.xml);
			dvr_data = xml2json.parser(data.xml, "", false)
			if(dvr_data.juan.envload.errno != 0)
			{
				alert_error(dvr_data.juan.envload.errno);
				return;
			}

			dvr_network_data2ui();
		},
		complete: function(XMLHttpRequest, textStatus){
//			alert("complete:" + textStatus)
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
		//	alert(textStatus);
		}
	});	
}
function dvr_network_save_content()
{
	var dvr_openDH = $('#dvr_openDH').prop('checked') ?1:0;
	var dvr_openDD = $("#dvr_openDD").prop('checked') ?1:0;
	var dvr_OpenPPPOE = $("#dvr_Open_PPPOE").prop('checked') ?1:0;
	var xmlstr = '';
	xmlstr += '<juan ver="0" squ="fastweb" dir="0">';
	xmlstr += '<envload type="1" usr="' + dvr_usr + '" pwd="' + dvr_pwd + '">';
	xmlstr += '<network';
	xmlstr += ' dhcp="' + dvr_openDH + '"';
	xmlstr += ' mac="' + $("#dvr_mac_addr")[0].value + '"';
	xmlstr += ' ip="' + $("#dvr_ip_addr")[0].value + '"';
	xmlstr += ' submask="' + $("#dvr_subnet_mask")[0].value + '"';
	xmlstr += ' gateway="' + $("#dvr_network_addr")[0].value + '"';
	xmlstr += ' dns="' + $("#dvr_dns_addr")[0].value + '"';
	xmlstr += ' httpport="' + $("#dvr_web_port")[0].value + '"';
	xmlstr += ' clientport="' + $("#dvr_client_port")[0].value + '"';
	xmlstr += ' enetid="' + $("#dvr_eseeID")[0].value + '"';
	xmlstr += ' ddns="' + dvr_openDD + '"';
	xmlstr += ' ddnsprovider="' + $("#dvr_supplier")[0].value + '"';
	xmlstr += ' ddnsurl="' + $("#dvr_domain")[0].value + '"';
	xmlstr += ' ddnsusr="' + $("#dvr_user_DD")[0].value + '"';
	xmlstr += ' ddnspwd="' + $("#dvr_password_DD")[0].value + '"';
	xmlstr += ' pppoe="' + dvr_OpenPPPOE + '"';
	xmlstr += ' pppoeusr="' + $("#dvr_user_PPPOE")[0].value + '"';
	xmlstr += ' pppoepwd="' + $("#dvr_password_PPPOE")[0].value + '"';
	xmlstr += ' />';
	xmlstr += '</envload>';
	xmlstr += '</juan>';
//	alert(xmlstr);

	dvr_ajax = $.ajax({ 
		type:"GET",
		url: dvr_url + "/cgi-bin/gw.cgi?f=j", 
		processData: false, 
		cache: false,
		data: "xml=" + xmlstr, 
		async:true,
		dataType: 'jsonp',
		jsonp: 'jsoncallback',

		beforeSend: function(XMLHttpRequest){
//			alert("beforeSend");
		},
		success: function(data, textStatus){
//			alert("recv:" + data.xml);

			var ret_data = xml2json.parser(data.xml, "", false)
			if(ret_data.juan.envload.errno != 0)
			{
				alert_error(ret_data.juan.envload.errno);
				return;
			}
		},
		complete: function(XMLHttpRequest, textStatus){
//			alert("complete:" + textStatus);
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
			//alert(textStatus);
		}
	});	
}
//encoding
function setAjax(xmlstr,fun){
	$.ajax({
		url: dvr_url + "/cgi-bin/gw.cgi?f=j", 
		processData: false,
		cache: false,
		async:false,
		dataType: 'jsonp',
		jsonp: 'jsoncallback',
		type:"GET",
		data: "xml=" + xmlstr, 
		beforeSend: function(XMLHttpRequest){

		},
		success:function(data, textStatus){
			var re = xml2json.parser(data.xml, "", false);
			if(fun.name == 'dvr_encoding_data2ui'){
				dvr_data = re;
			}
			if(re.juan.envload.errno != 0)
			{
				alert_error(re.juan.envload.errno);
			}
			fun(dvr_selected_chn);
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){	
			//alert(textStatus);
		},
		complete: function(XMLHttpRequest, textStatus){
//			alert("complete:" + textStatus);
		}
	})
}
function dvr_encoding_data2ui(dvr_selected_chn)
{
	$("#dvr_enc_chn_sel0").html(dvr_selected_chn+1);
	//main_stream
	$('#dvr_enc_main_mode_ID').val(dvr_data.juan.envload.encode[dvr_selected_chn].mode);
	switch(dvr_data.juan.envload.encode[dvr_selected_chn].mode)
	{
		case 0:	$("#dvr_enc_main_mode")[0].innerHTML = lang.Video_streaming;break;
		case 1:	$("#dvr_enc_main_mode")[0].innerHTML = lang.Audio_and_video_streaming;break;
		default: break;
	}
	$('#dvr_enc_main_mode_ID').nextAll('li').click(function(){
		 	$('#dvr_enc_main_mode_ID').val($(this).attr('value'));
		})
	switch(dvr_data.juan.envload.encode[dvr_selected_chn].fmt)
	{
		case 0:	$("#dvr_enc_main_format")[0].innerHTML = 'QCIP';break;
		case 1:	$("#dvr_enc_main_format")[0].innerHTML = 'CIF';break;
		case 2:	$("#dvr_enc_main_format")[0].innerHTML = 'HD1';break;
		case 3:	$("#dvr_enc_main_format")[0].innerHTML = 'D1';break;
		default: break;
	}
	if(dvr_data.juan.envload.encode[dvr_selected_chn].fmt >=4){
		$('#dvr_enc_main_format_sel').find('li').remove();
		var selc = $('<li><a href="javascript:;">QCIP</a></li><li><a href="javascript:;">CIF</a></li><li><a href="javascript:;">HD1</a></li><li><a href="javascript:;">D1</a></li><li><a href="javascript:;">WCIF</a></li><li><a href="javascript:;">960H</a></li>').appendTo('#dvr_enc_main_format_sel');
		$('#dvr_enc_main_format_sel li a').each(function(index) {
				$(this).click(function(){
					$('#dvr_enc_main_format').html($(this)[0].innerHTML)	
				})
			});
		}	
	$('#dvr_enc_main_image_ID').val(dvr_data.juan.envload.encode[dvr_selected_chn].piclv);
	switch(dvr_data.juan.envload.encode[dvr_selected_chn].piclv)
	{
		case 0:	$("#dvr_enc_main_image")[0].innerHTML = lang.Highest;break;
		case 1:	$("#dvr_enc_main_image")[0].innerHTML = lang.High;break;
		case 2:	$("#dvr_enc_main_image")[0].innerHTML = lang.Moderate;break;
		case 3:	$("#dvr_enc_main_image")[0].innerHTML = lang.Low;break;
		case 4:	$("#dvr_enc_main_image")[0].innerHTML = lang.Minimum;break;
		default: break;
	}
	$('#dvr_enc_main_image_ID').nextAll('li').click(function(){
		 	$('#dvr_enc_main_image_ID').val($(this).attr('value'));
		})
	$('#dvr_enc_main_bitmode_ID').val(dvr_data.juan.envload.encode[dvr_selected_chn].bitmode);
	switch(dvr_data.juan.envload.encode[dvr_selected_chn].bitmode)
	{
		case 0:	$("#dvr_enc_main_bitmode")[0].innerHTML = lang.Variable_rate;break;
		case 1:	$("#dvr_enc_main_bitmode")[0].innerHTML = lang.Fixed_rate;break;
		case 2:	$("#dvr_enc_main_bitmode")[0].innerHTML = lang.Moderate_rate;break;
		default: break;
	}
	$('#dvr_enc_main_bitmode_ID').nextAll('li').click(function(){
		 	$('#dvr_enc_main_bitmode_ID').val($(this).attr('value'));
		})
	switch(dvr_data.juan.envload.encode[dvr_selected_chn].bitvalue)
	{
		case 0:	$("#dvr_enc_main_bitvalue")[0].innerHTML = '64kbps';break;
		case 1:	$("#dvr_enc_main_bitvalue")[0].innerHTML = '128kbps';break;
		case 2:	$("#dvr_enc_main_bitvalue")[0].innerHTML = '256kbps';break;
		case 3:	$("#dvr_enc_main_bitvalue")[0].innerHTML = '384kbps';break;
		case 4:	$("#dvr_enc_main_bitvalue")[0].innerHTML = '512kbps';break;
		case 5:	$("#dvr_enc_main_bitvalue")[0].innerHTML = '768kbps';break;
		case 6:	$("#dvr_enc_main_bitvalue")[0].innerHTML = '1Mbps';break;
		case 7:	$("#dvr_enc_main_bitvalue")[0].innerHTML = '1.5Mbps';break;
		case 8:	$("#dvr_enc_main_bitvalue")[0].innerHTML = '2Mbps';break;
		default: break;
	}
	switch(dvr_data.juan.envload.encode[dvr_selected_chn].framerate)
	{
		case 0:	$("#dvr_enc_main_framerate")[0].innerHTML = '1FPS';break;
		case 1:	$("#dvr_enc_main_framerate")[0].innerHTML = '2FPS';break;
		case 2:	$("#dvr_enc_main_framerate")[0].innerHTML = '3FPS';break;
		case 3:	$("#dvr_enc_main_framerate")[0].innerHTML = '5FPS';break;
		case 4:	$("#dvr_enc_main_framerate")[0].innerHTML = '8FPS';break;
		case 5:	$("#dvr_enc_main_framerate")[0].innerHTML = '10FPS';break;
		case 6:	$("#dvr_enc_main_framerate")[0].innerHTML = '12FPS';break;
		case 7:	$("#dvr_enc_main_framerate")[0].innerHTML = '15FPS';break;
		case 8:	$("#dvr_enc_main_framerate")[0].innerHTML = '20FPS';break;
		case 9:	$("#dvr_enc_main_framerate")[0].innerHTML = '25FPS';break;
		default: break;
	}
	//sub_stream
	switch(dvr_data.juan.envload.encodesub[dvr_selected_chn].mode)
	{
		case 0:	$("#dvr_enc_sub_mode")[0].innerHTML = lang.Video_streaming;break;
		default: break;
	}
	switch(dvr_data.juan.envload.encodesub[dvr_selected_chn].fmt)
	{
		case 0:	$("#dvr_enc_sub_format")[0].innerHTML = 'QCIP';break;
		case 1:	$("#dvr_enc_sub_format")[0].innerHTML = 'CIF';break;
		default: break;
	}
	$('#dvr_enc_sub_image_ID').val(dvr_data.juan.envload.encodesub[dvr_selected_chn].piclv);
	switch(dvr_data.juan.envload.encodesub[dvr_selected_chn].piclv)
	{
		case 0:	$("#dvr_enc_sub_image")[0].innerHTML = lang.Highest;break;
		case 1:	$("#dvr_enc_sub_image")[0].innerHTML = lang.High;break;
		case 2:	$("#dvr_enc_sub_image")[0].innerHTML = lang.Moderate;break;
		case 3:	$("#dvr_enc_sub_image")[0].innerHTML = lang.Low;break;
		case 4:	$("#dvr_enc_sub_image")[0].innerHTML = lang.Minimum;break;
		default: break;
	}
	$('#dvr_enc_sub_image_ID').nextAll('li').click(function(){
		 	$('#dvr_enc_sub_image_ID').val($(this).attr('value'));
		})
	$('#dvr_enc_sub_bitmode_ID').val(dvr_data.juan.envload.encodesub[dvr_selected_chn].bitmode);
	switch(dvr_data.juan.envload.encodesub[dvr_selected_chn].bitmode)
	{
		case 0:	$("#dvr_enc_sub_bitmode")[0].innerHTML = lang.Variable_rate;break;
		case 1:	$("#dvr_enc_sub_bitmode")[0].innerHTML = lang.Fixed_rate;break;
		case 2:	$("#dvr_enc_sub_bitmode")[0].innerHTML = lang.Moderate_rate;break;
		default: break;
	}
	$('#dvr_enc_sub_bitmode_ID').nextAll('li').click(function(){
		 	$('#dvr_enc_sub_bitmode_ID').val($(this).attr('value'));
		})
	switch(dvr_data.juan.envload.encodesub[dvr_selected_chn].bitvalue)
	{
		case 0:	$("#dvr_enc_sub_bitvalue")[0].innerHTML = '64kbps';break;
		case 1:	$("#dvr_enc_sub_bitvalue")[0].innerHTML = '128kbps';break;
		case 2:	$("#dvr_enc_sub_bitvalue")[0].innerHTML = '256kbps';break;
		case 3:	$("#dvr_enc_sub_bitvalue")[0].innerHTML = '384kbps';break;
		default: break;
	}
	switch(dvr_data.juan.envload.encodesub[dvr_selected_chn].framerate)
	{
		case 0:	$("#dvr_enc_sub_framerate")[0].innerHTML = '1FPS';break;
		case 1:	$("#dvr_enc_sub_framerate")[0].innerHTML = '2FPS';break;
		case 2:	$("#dvr_enc_sub_framerate")[0].innerHTML = '3FPS';break;
		case 3:	$("#dvr_enc_sub_framerate")[0].innerHTML = '5FPS';break;
		case 4:	$("#dvr_enc_sub_framerate")[0].innerHTML = '8FPS';break;
		case 5:	$("#dvr_enc_sub_framerate")[0].innerHTML = '10FPS';break;
		case 6:	$("#dvr_enc_sub_framerate")[0].innerHTML = '12FPS';break;
		default: break;
	}
}
function dvr_encoding_load_content()
{
	var xmlstr = '';
	xmlstr += '<juan ver="0" squ="fastweb" dir="0">';
	xmlstr += '<envload type="0" usr="' + dvr_usr + '" pwd="' + dvr_pwd + '">';
	for(var i = 0; i < dvr_chn; i++)
	{
		xmlstr += '<encode chn="' + i + '" mode="" fmt="" piclv="" bitmode="" bitvalue="" framerate="" />';
		xmlstr += '<encodesub chn="' + i + '" mode="" fmt="" piclv="" bitmode="" bitvalue="" framerate="" />';
	}
	xmlstr += '</envload>';
	xmlstr += '</juan>';
	setAjax(xmlstr,dvr_encoding_data2ui);
//	alert(xmlstr);
}

function dvr_encoding_save_content(ret)
{
	//main_stream
	var chk_chn = $("#dvr_enc_chn_sel0")[0].innerHTML-1;
	var dvr_enc_main_format,dvr_enc_main_bitvalue,dvr_enc_main_framerate;
	switch($("#dvr_enc_main_format")[0].innerHTML)
	{
		case 'QCIP': dvr_enc_main_format = 0;break;
		case 'CIF':	dvr_enc_main_format = 1;break;
		case 'HD1':	dvr_enc_main_format = 2;break;
		case 'D1': dvr_enc_main_format = 3;break;
		case 'WCIF': dvr_enc_main_format = 4;break;
		case '960H': dvr_enc_main_format = 5;break;
		default: break;
	}
	switch($("#dvr_enc_main_bitvalue")[0].innerHTML)
	{
		case '64kbps': dvr_enc_main_bitvalue = 0;break;
		case '128kbps':	dvr_enc_main_bitvalue = 1;break;
		case '256kbps':	dvr_enc_main_bitvalue = 2;break;
		case '384kbps':	dvr_enc_main_bitvalue = 3;break;
		case '512kbps':	dvr_enc_main_bitvalue = 4;break;
		case '768kbps':	dvr_enc_main_bitvalue = 5;break;
		case '1Mbps': dvr_enc_main_bitvalue = 6;break;
		case '1.5Mbps':	dvr_enc_main_bitvalue = 7;break;
		case '2Mbps': dvr_enc_main_bitvalue = 8;break;
		default: break;
	}
	switch($("#dvr_enc_main_framerate")[0].innerHTML)
	{
		case '1FPS': dvr_enc_main_framerate = 0;break;
		case '2FPS': dvr_enc_main_framerate = 1;break;
		case '3FPS': dvr_enc_main_framerate = 2;break;
		case '5FPS': dvr_enc_main_framerate = 3;break;
		case '8FPS': dvr_enc_main_framerate = 4;break;
		case '10FPS': dvr_enc_main_framerate = 5;break;
		case '12FPS': dvr_enc_main_framerate = 6;break;
		case '15FPS': dvr_enc_main_framerate = 7;break;
		case '20FPS': dvr_enc_main_framerate = 8;break;
		case '25FPS': dvr_enc_main_framerate = 9;break;
		default: break;
	}
	//sub_stream
	var dvr_enc_sub_format,dvr_enc_sub_bitvalue,dvr_enc_sub_framerate;
	switch($("#dvr_enc_sub_format")[0].innerHTML)
	{
		case 'QCIP': dvr_enc_sub_format = 0;break;
		case 'CIF':	dvr_enc_sub_format = 1;break;
		default: break;
	}
	switch($("#dvr_enc_sub_bitvalue")[0].innerHTML)
	{
		case '64kbps': dvr_enc_sub_bitvalue = 0;break;
		case '128kbps':	dvr_enc_sub_bitvalue = 1;break;
		case '256kbps':	dvr_enc_sub_bitvalue = 2;break;
		case '384kbps':	dvr_enc_sub_bitvalue = 3;break;
		default: break;
	}
	switch($("#dvr_enc_sub_framerate")[0].innerHTML)
	{
		case '1FPS': dvr_enc_sub_framerate = 0;break;
		case '2FPS': dvr_enc_sub_framerate = 1;break;
		case '3FPS': dvr_enc_sub_framerate = 2;break;
		case '5FPS': dvr_enc_sub_framerate = 3;break;
		case '8FPS': dvr_enc_sub_framerate = 4;break;
		case '10FPS': dvr_enc_sub_framerate = 5;break;
		case '12FPS': dvr_enc_sub_framerate = 6;break;
		default: break;
	}
	var xmlstr = '';
	xmlstr += '<juan ver="0" squ="fastweb" dir="0">';
	xmlstr += '<envload type="1" usr="' + dvr_usr + '" pwd="' + dvr_pwd + '">';
	xmlstr += '<encode';
	xmlstr += ' chn="' + chk_chn + '"';
	xmlstr += ' mode="' + $("#dvr_enc_main_mode_ID")[0].value + '"';
	xmlstr += ' fmt="' + dvr_enc_main_format + '"';
	xmlstr += ' piclv="' + $("#dvr_enc_main_image_ID")[0].value + '"';
	xmlstr += ' bitmode="' + $("#dvr_enc_main_bitmode_ID")[0].value + '"';
	xmlstr += ' bitvalue="' + dvr_enc_main_bitvalue + '"';
	xmlstr += ' framerate="' + dvr_enc_main_framerate + '"';
	xmlstr += ' />';
	xmlstr += '<encodesub';
	xmlstr += ' chn="' + chk_chn + '"';
	xmlstr += ' mode="' + $("#dvr_enc_sub_mode_ID")[0].value + '"';
	xmlstr += ' fmt="' + dvr_enc_sub_format + '"';
	xmlstr += ' piclv="' + $("#dvr_enc_sub_image_ID")[0].value + '"';
	xmlstr += ' bitmode="' + $("#dvr_enc_sub_bitmode_ID")[0].value + '"';
	xmlstr += ' bitvalue="' + dvr_enc_sub_bitvalue + '"';
	xmlstr += ' framerate="' + dvr_enc_sub_framerate + '"';
	xmlstr += ' />';
	if(ret > 0)
	{
		xmlstr += '<copyg';
		xmlstr += ' chn="' + chk_chn + '"';
		xmlstr += ' type="1"';
		xmlstr += ' channels="' + ret + '"';
		xmlstr += ' />';
	}
	if(ret > 0)
	{
		xmlstr += '<copyg';
		xmlstr += ' chn="' + chk_chn + '"';
		xmlstr += ' type="6"';
		xmlstr += ' channels="' + ret + '"';
		xmlstr += ' />';
	}
	xmlstr += '</envload>';
	xmlstr += '</juan>';
	//alert(xmlstr);
	dvr_ajax = $.ajax({ 
		type:"GET",
		url: dvr_url + "/cgi-bin/gw.cgi?f=j", 
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
			var ret_data = xml2json.parser(data.xml, "", false)

			if(ret_data.juan.envload.errno != 0)
			{
				alert_error(ret_data.juan.envload.errno);
				return;
			}
		},
		complete: function(XMLHttpRequest, textStatus){
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
			//alert(textStatus);
		}
	});	
}

//record or(video)
function dvr_record_data2ui(dvr_selected_chn,dvr_selected_weekday)
{
	$("#dvr_record_chn_sel0").html(dvr_selected_chn+1);
	$('#dvr_record_week_ID').val(dvr_selected_weekday);
	switch(dvr_selected_weekday){
		case 0:$("#dvr_record_week")[0].innerHTML = lang.Sun;break;
		case 1:$("#dvr_record_week")[0].innerHTML = lang.Mon;break;
		case 2:$("#dvr_record_week")[0].innerHTML = lang.Tue;break;
		case 3:$("#dvr_record_week")[0].innerHTML = lang.Wed;break;
		case 4:$("#dvr_record_week")[0].innerHTML = lang.Thu;break;
		case 5:$("#dvr_record_week")[0].innerHTML = lang.Fri;break;
		case 6:$("#dvr_record_week")[0].innerHTML = lang.Sat;break;
		default:break;
		}
	$('#dvr_record_weekday_'+dvr_selected_weekday).prop('checked',true);
	for(var i = 0; i < 4; i++)
	{
		$("#dvr_record_begin_" + i)[0].value = dvr_data.juan.envload.record[i].begin;
		$("#dvr_record_end_" + i)[0].value = dvr_data.juan.envload.record[i].end;
		var types = parseInt(dvr_data.juan.envload.record[i].types);
		for(var j = 0; j < 3; j++)
		{
			if(types == 0)
			{
				$("#dvr_record_types_" + i + "_" + j)[0].checked = false;
			}
			else
			{
				$("#dvr_record_types_" + i + "_" + j)[0].checked = ((types & (1 << j)) == types);
			}
		}
	}
}
function dvr_record_load_content(dvr_selected_chn,dvr_selected_weekday)
{
	$('#dvr_record_week_sel').find('li').remove();
	for(var i = 0; i < 7; i++)
	{
		var week;
		switch(i){
			case 0:week = lang.Sun;break;break;
			case 1:week = lang.Mon;break;break;
			case 2:week = lang.Tue;break;break;
			case 3:week = lang.Wed;break;break;
			case 4:week = lang.Thu;break;break;
			case 5:week = lang.Fri;break;break;
			case 6:week = lang.Sat;break;break;
			default:break;
		}
		$('#dvr_record_week_sel').append('<li value="'+i+'" class="add_li_2"><a href="javascript:;">'+week+'</a></li>');
	}
	$('.add_li_2 a').each(function(index) {
			$(this).click(function(){
						$('#dvr_record_week_ID').val(index);
						switch(dvr_selected_chn){
							case 0:$("#dvr_record_week")[0].innerHTML = lang.Sun;break;
							case 1:$("#dvr_record_week")[0].innerHTML = lang.Mon;break;
							case 2:$("#dvr_record_week")[0].innerHTML = lang.Tue;break;
							case 3:$("#dvr_record_week")[0].innerHTML = lang.Wed;break;
							case 4:$("#dvr_record_week")[0].innerHTML = lang.Thu;break;
							case 5:$("#dvr_record_week")[0].innerHTML = lang.Fri;break;
							case 6:$("#dvr_record_week")[0].innerHTML = lang.Sat;break;
							default:break;
						}				
						$('.chk').prop('checked',false);
						$('#dvr_record_weekday_'+index).prop('checked',true);
						dvr_record_load_content(dvr_selected_chn,index);
				})
		});
	var xmlstr = '';
	xmlstr += '<juan ver="0" squ="fastweb" dir="0">';
	xmlstr += '<envload type="0" usr="' + dvr_usr + '" pwd="' + dvr_pwd + '">';
	for(var i = 0; i < 4; i ++)
	{
		xmlstr += '<record chn="' + dvr_selected_chn + '" weekday="' + dvr_selected_weekday + '" index="' + i + '" begin="" end="" types="" />';
	}
	xmlstr += '</envload>';
	xmlstr += '</juan>';
//	alert(xmlstr);

	dvr_ajax = $.ajax({ 
		type:"GET",
		url: dvr_url + "/cgi-bin/gw.cgi?f=j", 
		processData: false, 
		cache: false,
		data: "xml=" + xmlstr, 
		async:true,
		dataType: 'jsonp',
		jsonp: 'jsoncallback',

		beforeSend: function(XMLHttpRequest){
//			alert("beforeSend");
		},
		success: function(data, textStatus){
//			alert("recv:" + data.xml);
			dvr_data = xml2json.parser(data.xml, "", false)

			if(dvr_data.juan.envload.errno != 0)
			{
				alert_error(dvr_data.juan.envload.errno);
				return;
			}

			dvr_record_data2ui(dvr_selected_chn,dvr_selected_weekday);
		},
		complete: function(XMLHttpRequest, textStatus){
//			alert("complete:" + textStatus);
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
			//alert(textStatus);
		}
	});	
}

function dvr_record_save_content(ret)
{
	var chk_chn = $('#dvr_record_chn_sel0').html()-1;
	var weekday = $('#dvr_record_week_ID').val();
	var xmlstr = '';
	xmlstr += '<juan ver="0" squ="fastweb" dir="0">';
	xmlstr += '<envload type="1" usr="' + dvr_usr + '" pwd="' + dvr_pwd + '">';
	for(var i = 0; i < 4; i++)
	{
		xmlstr += '<record';
		xmlstr += ' chn="' + chk_chn + '"';
		xmlstr += ' weekday="' + weekday + '"';
		xmlstr += ' index="' + i + '"';
		xmlstr += ' begin="' + $("#dvr_record_begin_" + i)[0].value + '"';
		xmlstr += ' end="' + $("#dvr_record_end_" + i)[0].value + '"';
		var types = 0;
		for(var j = 0; j < 3; j++)
		{
			if($("#dvr_record_types_" + i + "_" + j)[0].checked == true)
			{
				types += Math.pow(2, j);
			}
		}
		xmlstr += ' types="' + types + '"';
		xmlstr += ' />';
	}
	if(ret > 0)
	{
		xmlstr += '<copyrec';
		xmlstr += ' chn="' + chk_chn + '"';
		xmlstr += ' weekday="' + weekday + '"';
		xmlstr += ' channels="' + ret + '"';
		xmlstr += ' weekdays="' + (1 << weekday) + '"';
		xmlstr += ' />';
	}
	xmlstr += '</envload>';
	xmlstr += '</juan>';
	//alert(xmlstr);

	dvr_ajax = $.ajax({ 
		type:"GET",
		url: dvr_url + "/cgi-bin/gw.cgi?f=j", 
		processData: false, 
		cache: false,
		data: "xml=" + xmlstr, 
		async:true,
		dataType: 'jsonp',
		jsonp: 'jsoncallback',
		beforeSend: function(XMLHttpRequest){
//			alert("beforeSend");
		},
		success: function(data, textStatus){
//			alert("recv:" + data.xml);
			var ret_data = xml2json.parser(data.xml, "", false)

			if(ret_data.juan.envload.errno != 0)
			{
				alert_error(ret_data.juan.envload.errno);
				return;
			}
		},
		complete: function(XMLHttpRequest, textStatus){
//			alert("complete:" + textStatus);
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
			//alert(textStatus);
		}
	});	
}
//screen
function dvr_screen_data2ui(dvr_selected_chn)
{
	$("#dvr_screen_chn_sel0").html(dvr_selected_chn+1);
	$("#dvr_screen_name")[0].value = dvr_data.juan.envload.screen[dvr_selected_chn].title;
}
function dvr_screen_load_content()
{
	var xmlstr = '';
	xmlstr += '<juan ver="0" squ="fastweb" dir="0">';
	xmlstr += '<envload type="0" usr="' + dvr_usr + '" pwd="' + dvr_pwd + '">';
	for(var i = 0; i < dvr_chn; i++)
	{
		xmlstr += '<screen chn="' + i + '" title=""/>';
	}
	xmlstr += '</envload>';
	xmlstr += '</juan>';
//	alert(xmlstr);

	dvr_ajax = $.ajax({ 
		type:"GET",
		url: dvr_url + "/cgi-bin/gw.cgi?f=j", 
		processData: false, 
		cache: false,
		data: "xml=" + xmlstr, 
		async:true,
		dataType: 'jsonp',
		jsonp: 'jsoncallback',

		beforeSend: function(XMLHttpRequest){
//			alert("beforeSend");
		},
		success: function(data, textStatus){
//			alert("recv:" + data.xml);
			dvr_data = xml2json.parser(data.xml, "", false)

			if(dvr_data.juan.envload.errno != 0)
			{
				alert_error(dvr_data.juan.envload.errno);
				return;
			}

			dvr_screen_data2ui(dvr_selected_chn);
		},
		complete: function(XMLHttpRequest, textStatus){
//			alert("complete:" + textStatus);
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
			//alert(textStatus);
		}
	});	
}
function dvr_screen_save_content()
{
	var chk_chn = $("#dvr_screen_chn_sel0").html()-1;
	var xmlstr = '';
	xmlstr += '<juan ver="0" squ="fastweb" dir="0" enc="1">';
	xmlstr += '<envload type="1" usr="' + dvr_usr + '" pwd="' + dvr_pwd + '">';
	xmlstr += '<screen';
	xmlstr += ' chn="' + chk_chn + '"';
	xmlstr += ' title="' + $("#dvr_screen_name")[0].value + '"';
	xmlstr += ' />';
	xmlstr += '</envload>';
	xmlstr += '</juan>';
	//alert(xmlstr);

	dvr_ajax = $.ajax({ 
		type:"GET",
		url: dvr_url + "/cgi-bin/gw.cgi?f=j", 
		processData: false, 
		cache: false,
		data: "xml=" + encodeURI(xmlstr), 
		async:true,
		dataType: 'jsonp',
		jsonp: 'jsoncallback',

		beforeSend: function(XMLHttpRequest){
//			alert("beforeSend");
		},
		success: function(data, textStatus){
//			alert("recv:" + data.xml);
			var ret_data = xml2json.parser(data.xml, "", false)

			if(ret_data.juan.envload.errno != 0)
			{
				alert_error(ret_data.juan.envload.errno);
				return;
			}
		},
		complete: function(XMLHttpRequest, textStatus){
//			alert("complete:" + textStatus);
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
			//alert(textStatus);
		}
	});	
}
//detect
function dvr_detect_data2ui(dvr_selected_chn)
{
	$("#dvr_detect_chn_sel0").html(dvr_selected_chn+1);
	$('#dvr_detect_sens_ID').val(dvr_data.juan.envload.detection[dvr_selected_chn].sens);
	switch(dvr_data.juan.envload.detection[dvr_selected_chn].sens)
	{
		case 0:	$("#dvr_detect_sens")[0].innerHTML = lang.Highest;break;
		case 1:	$("#dvr_detect_sens")[0].innerHTML = lang.High;break;
		case 2:	$("#dvr_detect_sens")[0].innerHTML = lang.Moderate;break;
		case 3:	$("#dvr_detect_sens")[0].innerHTML = lang.Low;break;
		case 4:	$("#dvr_detect_sens")[0].innerHTML = lang.Minimum;break;
		default: break;
	}
	$('#dvr_detect_sens_ID').nextAll('li').click(function(){
		 	$('#dvr_detect_sens_ID').val($(this).attr('value'));
		})
	$('#dvr_detect_mduration_ID').val(dvr_data.juan.envload.detection[dvr_selected_chn].mdalarmduration);
	switch(dvr_data.juan.envload.detection[dvr_selected_chn].mdalarmduration)
	{
		case 0:	$("#dvr_detect_mduration")[0].innerHTML = '1s';break;
		case 1:	$("#dvr_detect_mduration")[0].innerHTML = '2s';break;
		case 2:	$("#dvr_detect_mduration")[0].innerHTML = '3s';break;
		case 3:	$("#dvr_detect_mduration")[0].innerHTML = '4s';break;
		case 4:	$("#dvr_detect_mduration")[0].innerHTML = '5s';break;
		case 5:	$("#dvr_detect_mduration")[0].innerHTML = '8s';break;
		case 6:	$("#dvr_detect_mduration")[0].innerHTML = '10s';break;
		case 7:	$("#dvr_detect_mduration")[0].innerHTML = lang.Continuous;break;
		default: break;
	}
	$('#dvr_detect_mduration_ID').nextAll('li').click(function(){
		 	$('#dvr_detect_mduration_ID').val($(this).attr('value'));
		})
	if(dvr_data.juan.envload.detection[dvr_selected_chn].mdalarm == 1){
		$('#dvr_detect_mAlarm').prop('checked',true);
	}else{
		$('#dvr_detect_mAlarm').prop('checked',false);
	}
	if(dvr_data.juan.envload.detection[dvr_selected_chn].mdbuzzer == 1){
		$('#dvr_detect_mBuzzer').prop('checked',true);
	}else{
		$('#dvr_detect_mBuzzer').prop('checked',false);
	}
	$('#dvr_detect_vduration_ID').val(dvr_data.juan.envload.detection[dvr_selected_chn].vlalarmduration);
	switch(dvr_data.juan.envload.detection[dvr_selected_chn].vlalarmduration)
	{
		case 0:	$("#dvr_detect_vduration")[0].innerHTML = '1s';break;
		case 1:	$("#dvr_detect_vduration")[0].innerHTML = '2s';break;
		case 2:	$("#dvr_detect_mduration")[0].innerHTML = '3s';break;
		case 3:	$("#dvr_detect_vduration")[0].innerHTML = '4s';break;
		case 4:	$("#dvr_detect_vduration")[0].innerHTML = '5s';break;
		case 5:	$("#dvr_detect_vduration")[0].innerHTML = '8s';break;
		case 6:	$("#dvr_detect_vduration")[0].innerHTML = '10s';break;
		case 7:	$("#dvr_detect_vduration")[0].innerHTML = lang.Continuous;break;
		default: break;
	}
	$('#dvr_detect_vduration_ID').nextAll('li').click(function(){
		 	$('#dvr_detect_vduration_ID').val($(this).attr('value'));
		})
	if(dvr_data.juan.envload.detection[dvr_selected_chn].vlalarm == 1){
		$('#dvr_detect_vAlarm').prop('checked',true);
	}else{
		$('#dvr_detect_vAlarm').prop('checked',false);
	}
	if(dvr_data.juan.envload.detection[dvr_selected_chn].vlbuzzer == 1){
		$('#dvr_detect_vBuzzer').prop('checked',true);
	}else{
		$('#dvr_detect_vBuzzer').prop('checked',false);
	}
}
function dvr_detect_load_content()
{
	var xmlstr = '';
	xmlstr += '<juan ver="0" squ="fastweb" dir="0">';
	xmlstr += '<envload type="0" usr="' + dvr_usr + '" pwd="' + dvr_pwd + '">';
	for(var i = 0; i < dvr_chn; i++)
	{
		xmlstr += '<detection chn="' + i + '" sens="" mdalarmduration="" mdalarm="" mdbuzzer="" vlalarmduration="" vlalarm="" vlbuzzer="" />';
	}
	xmlstr += '</envload>';
	xmlstr += '</juan>';
//	alert(xmlstr);

	dvr_ajax = $.ajax({ 
		type:"GET",
		url: dvr_url + "/cgi-bin/gw.cgi?f=j", 
		processData: false, 
		cache: false,
		data: "xml=" + xmlstr, 
		async:true,
		dataType: 'jsonp',
		jsonp: 'jsoncallback',

		beforeSend: function(XMLHttpRequest){
//			alert("beforeSend");
		},
		success: function(data, textStatus){
//			alert("recv:" + data.xml);
			dvr_data = xml2json.parser(data.xml, "", false)

			if(dvr_data.juan.envload.errno != 0)
			{
				alert_error(dvr_data.juan.envload.errno);
				return;
			}
			dvr_detect_data2ui(dvr_selected_chn);
		},
		complete: function(XMLHttpRequest, textStatus){
//			alert("complete:" + textStatus);
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
			//alert(textStatus);
		}
	});	
}

function dvr_detect_save_content(ret)
{
	var chk_chn = $('#dvr_detect_chn_sel0').html()-1;
	var dvr_detect_sens,dvr_detect_mduration,dvr_detect_vduration;
	var dvr_detect_mAlarm = $('#dvr_detect_mAlarm').prop('checked') ?1:0;
	var dvr_detect_mBuzzer = $("#dvr_detect_mBuzzer").prop('checked') ?1:0;
	var dvr_detect_vAlarm = $('#dvr_detect_vAlarm').prop('checked') ?1:0;
	var dvr_detect_vBuzzer = $("#dvr_detect_vBuzzer").prop('checked') ?1:0;
	//

	var xmlstr = '';
	xmlstr += '<juan ver="0" squ="fastweb" dir="0">';
	xmlstr += '<envload type="1" usr="' + dvr_usr + '" pwd="' + dvr_pwd + '">';
	xmlstr += '<detection';
	xmlstr += ' chn="' + chk_chn + '"';
	xmlstr += ' sens="' + $("#dvr_detect_sens_ID")[0].value + '"';
	xmlstr += ' mdalarmduration="' + $("#dvr_detect_mduration_ID")[0].value + '"';
	xmlstr += ' mdalarm="' + dvr_detect_mAlarm + '"';
	xmlstr += ' mdbuzzer="' + dvr_detect_mBuzzer + '"';
	xmlstr += ' vlalarmduration="' + $("#dvr_detect_vduration_ID")[0].value + '"';
	xmlstr += ' vlalarm="' + dvr_detect_vAlarm + '"';
	xmlstr += ' vlbuzzer="' + dvr_detect_vBuzzer + '"';
	xmlstr += ' />';
	if(ret > 0)
	{
		xmlstr += '<copyg';
		xmlstr += ' chn="' + chk_chn + '"';
		xmlstr += ' type="2"';
		xmlstr += ' channels="' + ret + '"';
		xmlstr += ' />';
	}
	xmlstr += '</envload>';
	xmlstr += '</juan>';
	//alert(xmlstr);

	dvr_ajax = $.ajax({ 
		type:"GET",
		url: dvr_url + "/cgi-bin/gw.cgi?f=j", 
		processData: false, 
		cache: false,
		data: "xml=" + xmlstr, 
		async:true,
		dataType: 'jsonp',
		jsonp: 'jsoncallback',

		beforeSend: function(XMLHttpRequest){
//			alert("beforeSend");
		},
		success: function(data, textStatus){
//			alert("recv:" + data.xml);
			var ret_data = xml2json.parser(data.xml, "", false)

			if(ret_data.juan.envload.errno != 0)
			{
				alert_error(ret_data.juan.envload.errno);
				return;
			}
		},
		complete: function(XMLHttpRequest, textStatus){
//			alert("complete:" + textStatus);
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
			//alert(textStatus);
		}
	});	
}
//ptz
function dvr_ptz_data2ui(dvr_selected_chn)
{
	$("#dvr_ptz_chn_sel0").html(dvr_selected_chn+1);
	$('#dvr_ptz_device_addr')[0].value = dvr_data.juan.envload.ptz[dvr_selected_chn].id;
	$('#dvr_ptz_agreement_ID').val(dvr_data.juan.envload.ptz[dvr_selected_chn].protocal);
	switch(dvr_data.juan.envload.ptz[dvr_selected_chn].protocal)
	{
		case 0: $("#dvr_ptz_agreement")[0].innerHTML = 'pelco-D';break;
		case 1: $("#dvr_ptz_agreement")[0].innerHTML = 'pelco-P';break;
		default: break;
	}
	$('#dvr_ptz_agreement_ID').nextAll('li').click(function(){
		 	$('#dvr_ptz_agreement_ID').val($(this).attr('value'));
		})
	switch(dvr_data.juan.envload.ptz[dvr_selected_chn].baudrate)
	{
		case 0: $("#dvr_ptz_baud_rate")[0].innerHTML = '2400';break;
		case 1: $("#dvr_ptz_baud_rate")[0].innerHTML = '4800';break;
		case 2: $("#dvr_ptz_baud_rate")[0].innerHTML = '9600';break;
		case 3: $("#dvr_ptz_baud_rate")[0].innerHTML = '19200';break;
		case 4: $("#dvr_ptz_baud_rate")[0].innerHTML = '38400';break;
		case 5: $("#dvr_ptz_baud_rate")[0].innerHTML = '57600';break;
		case 6: $("#dvr_ptz_baud_rate")[0].innerHTML = '115200';break;
		default: break;
	}
}

function dvr_ptz_ui2data()
{
	dvr_data.juan.envload.ptz[dvr_selected_chn].id = $("#juan_envload\\#ptz\\@id")[0].value;
	dvr_data.juan.envload.ptz[dvr_selected_chn].protocal = $("#juan_envload\\#ptz\\@protocal")[0].selectedIndex;
	dvr_data.juan.envload.ptz[dvr_selected_chn].baudrate = $("#juan_envload\\#ptz\\@baudrate")[0].selectedIndex;
	
	for(var i = 0; i < dvr_chn; i++)
	{
		if($("#chk_copyto_" + i)[0].checked == true)
		{
			dvr_data.juan.envload.ptz[i].id = $("#juan_envload\\#ptz\\@id")[0].value;
			dvr_data.juan.envload.ptz[i].protocal = $("#juan_envload\\#ptz\\@protocal")[0].selectedIndex;
			dvr_data.juan.envload.ptz[i].baudrate = $("#juan_envload\\#ptz\\@baudrate")[0].selectedIndex;
		}
	}
}

function dvr_ptz_load_content()
{
	var xmlstr = '';
	xmlstr += '<juan ver="0" squ="fastweb" dir="0">';
	xmlstr += '<envload type="0" usr="' + dvr_usr + '" pwd="' + dvr_pwd + '">';
	for(var i = 0; i < dvr_chn; i++)
	{
		xmlstr += '<ptz chn="' + i + '" id="" protocal="" baudrate="" />';
	}
	xmlstr += '</envload>';
	xmlstr += '</juan>';
//	alert(xmlstr);

	dvr_ajax = $.ajax({ 
		type:"GET",
		url: dvr_url + "/cgi-bin/gw.cgi?f=j", 
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
			dvr_data = xml2json.parser(data.xml, "", false)

			if(dvr_data.juan.envload.errno != 0)
			{
				alert_error(dvr_data.juan.envload.errno);
				return;
			}
			dvr_ptz_data2ui(dvr_selected_chn);
		},
		complete: function(XMLHttpRequest, textStatus){
//			alert("complete:" + textStatus)
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
			//alert(textStatus);
		}
	});	
}
function dvr_ptz_save_content(ret)
{
	var chk_chn = $('#dvr_ptz_chn_sel0').html()-1;
	var dvr_ptz_baud_rate;
	switch($("#dvr_ptz_baud_rate")[0].innerHTML)
	{
		case '2400': dvr_ptz_baud_rate = 0;break;
		case '4800': dvr_ptz_baud_rate = 1;break;
		case '9600': dvr_ptz_baud_rate = 2;break;
		case '19200': dvr_ptz_baud_rate = 3;break;
		case '38400': dvr_ptz_baud_rate = 4;break;
		case '57600': dvr_ptz_baud_rate = 5;break;
		case '115200': dvr_ptz_baud_rate = 6;break;
		default: break;
	}

	var xmlstr = '';
	xmlstr += '<juan ver="0" squ="fastweb" dir="0">';
	xmlstr += '<envload type="1" usr="' + dvr_usr + '" pwd="' + dvr_pwd + '">';
	xmlstr += '<ptz';
	xmlstr += ' chn="' + chk_chn + '"';
	xmlstr += ' id="' + $('#dvr_ptz_device_addr')[0].value + '"';
	xmlstr += ' protocal="' + $('#dvr_ptz_agreement_ID').val() + '"';
	xmlstr += ' baudrate="' + dvr_ptz_baud_rate + '"';
	xmlstr += ' />';
	if(ret > 0)
	{
		xmlstr += '<copyg';
		xmlstr += ' chn="' + chk_chn + '"';
		xmlstr += ' type="3"';
		xmlstr += ' channels="' + ret + '"';
		xmlstr += ' />';
	}
	xmlstr += '</envload>';
	xmlstr += '</juan>';
	//alert(xmlstr);

	dvr_ajax = $.ajax({ 
		type:"GET",
		url: dvr_url + "/cgi-bin/gw.cgi?f=j", 
		processData: false, 
		cache: false,
		data: "xml=" + xmlstr, 
		async:true,
		dataType: 'jsonp',
		jsonp: 'jsoncallback',

		beforeSend: function(XMLHttpRequest){
//			alert("beforeSend");
		},
		success: function(data, textStatus){
//			alert("recv:" + data.xml);
			var ret_data = xml2json.parser(data.xml, "", false)

			if(ret_data.juan.envload.errno != 0)
			{
				alert_error(ret_data.juan.envload.errno);
				return;
			}
		},
		complete: function(XMLHttpRequest, textStatus){
//			alert("complete:" + textStatus);
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
			//alert(textStatus);
		}
	});	
}
//alarm
function dvr_alarm_data2ui(dvr_selected_chn)
{
	$("#dvr_alarm_chn_sel0").html(dvr_selected_chn+1);
	if(dvr_data.juan.envload.sensor[dvr_selected_chn].alarm == 1){
		$('#dvr_alarm_Alarm').prop('checked',true);
	}else{
		$('#dvr_alarm_Alarm').prop('checked',false);
	}
	if(dvr_data.juan.envload.sensor[dvr_selected_chn].buzzer == 1){
		$('#dvr_alarm_Buzzer').prop('checked',true);
	}else{
		$('#dvr_alarm_Buzzer').prop('checked',false);
	}
	$('#dvr_alarm_opermode_ID').val(dvr_data.juan.envload.sensor[dvr_selected_chn].mode);
	switch(dvr_data.juan.envload.sensor[dvr_selected_chn].mode)
	{
		case 0:	$("#dvr_alarm_opermode")[0].innerHTML = lang.Close;break;
		case 1:	$("#dvr_alarm_opermode")[0].innerHTML = lang.Normally_open;break;
		case 2:	$("#dvr_alarm_opermode")[0].innerHTML = lang.Normally_closed;break;
		default: break;
	}
	$('#dvr_alarm_opermode_ID').nextAll('li').click(function(){
		 	$('#dvr_alarm_opermode_ID').val($(this).attr('value'));
		})
	$('#dvr_alarm_duration_ID').val(dvr_data.juan.envload.sensor[dvr_selected_chn].alarmduration);
	switch(dvr_data.juan.envload.sensor[dvr_selected_chn].alarmduration)
	{
		case 0:	$("#dvr_alarm_duration")[0].innerHTML = '1s';break;
		case 1:	$("#dvr_alarm_duration")[0].innerHTML = '2s';break;
		case 2:	$("#dvr_alarm_duration")[0].innerHTML = '3s';break;
		case 3:	$("#dvr_alarm_duration")[0].innerHTML = '4s';break;
		case 4:	$("#dvr_alarm_duration")[0].innerHTML = '5s';break;
		case 5:	$("#dvr_alarm_duration")[0].innerHTML = '8s';break;
		case 6:	$("#dvr_alarm_duration")[0].innerHTML = '10s';break;
		case 7:	$("#dvr_alarm_duration")[0].innerHTML = lang.Continuous;break;
		default: break;
	}
	$('#dvr_alarm_duration_ID').nextAll('li').click(function(){
		 	$('#dvr_alarm_duration_ID').val($(this).attr('value'));
		})
}
function dvr_alarm_load_content()
{
	var xmlstr = '';
	xmlstr += '<juan ver="0" squ="fastweb" dir="0">';
	xmlstr += '<envload type="0" usr="' + dvr_usr + '" pwd="' + dvr_pwd + '">';
	for(var i = 0; i < dvr_chn; i++)
	{
		xmlstr += '<sensor chn="' + i + '" mode="" alarmduration="" alarm="" buzzer="" />';
	}
	xmlstr += '</envload>';
	xmlstr += '</juan>';
//	alert(xmlstr);

	dvr_ajax = $.ajax({ 
		type:"GET",
		url: dvr_url + "/cgi-bin/gw.cgi?f=j", 
		processData: false, 
		cache: false,
		data: "xml=" + xmlstr, 
		async:true,
		dataType: 'jsonp',
		jsonp: 'jsoncallback',

		beforeSend: function(XMLHttpRequest){
//			alert("beforeSend")
		},
		success: function(data, textStatus){
//			alert("recv:" + data.xml);
			dvr_data = xml2json.parser(data.xml, "", false)

			if(dvr_data.juan.envload.errno != 0)
			{
				alert_error(dvr_data.juan.envload.errno);
				return;
			}
		dvr_alarm_data2ui(dvr_selected_chn);
		},
		complete: function(XMLHttpRequest, textStatus){
//			alert("complete:" + textStatus);
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
			//alert(textStatus);
		}
	});	
}

function dvr_alarm_save_content(ret)
{
	var chk_chn = $('#dvr_alarm_chn_sel0').html()-1;
	var dvr_alarm_opermode,dvr_alarm_duration;
	var dvr_alarm_Alarm = $('#dvr_alarm_Alarm').prop('checked') ?1:0;
	var dvr_alarm_Buzzer = $("#dvr_alarm_Buzzer").prop('checked') ?1:0;

	var xmlstr = '';
	xmlstr += '<juan ver="0" squ="fastweb" dir="0">';
	xmlstr += '<envload type="1" usr="' + dvr_usr + '" pwd="' + dvr_pwd + '">';
	xmlstr += '<sensor';
	xmlstr += ' chn="' + chk_chn + '"';
	xmlstr += ' mode="' + $("#dvr_alarm_opermode_ID")[0].value + '"';
	xmlstr += ' alarmduration="' + $("#dvr_alarm_duration_ID")[0].value + '"';
	xmlstr += ' alarm="' + dvr_alarm_Alarm + '"';
	xmlstr += ' buzzer="' + dvr_alarm_Buzzer + '"';
	xmlstr += ' />';
	if(ret > 0)
	{
		xmlstr += '<copyg';
		xmlstr += ' chn="' + chk_chn + '"';
		xmlstr += ' type="4"';
		xmlstr += ' channels="' + ret + '"';
		xmlstr += ' />';
	}
	xmlstr += '</envload>';
	xmlstr += '</juan>';
	//alert(xmlstr);

	dvr_ajax = $.ajax({ 
		type:"GET",
		url: dvr_url + "/cgi-bin/gw.cgi?f=j", 
		processData: false, 
		cache: false,
		data: "xml=" + xmlstr, 
		async:true,
		dataType: 'jsonp',
		jsonp: 'jsoncallback',

		beforeSend: function(XMLHttpRequest){
//			alert("beforeSend");
		},
		success: function(data, textStatus){
//			alert("recv:" + data.xml);
			var ret_data = xml2json.parser(data.xml, "", false)

			if(ret_data.juan.envload.errno != 0)
			{
				alert_error(ret_data.juan.envload.errno);
				return;
			}
		},
		complete: function(XMLHttpRequest, textStatus){
//			alert("complete:" + textStatus);
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
			//alert(textStatus);
		}
	});	
}
