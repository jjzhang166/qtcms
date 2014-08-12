var DVR = function(usr,pwd,ip,port,id,type){
	this._IP = ip;  //ip地址
	this._PORT = port; //端口
	this._USR = usr; //用户名
	this._PWD = pwd;  //密码
	this._ID = id;  //设备ID
	this._CHN = 0;//通道数
	this._TYPE = type; 
	this._VER = 0;  //设备版本号
	this._Upgrade = '';  // CMS 支持的最低版本IPC
    var _Request=[];  //判断数据加载与提示信息出现顺序是否合理的全局数组
	auth = "Basic " + base64.encode(this._USR+':'+this._PWD);  //用户信息，base64加密
	
	this.getRequestURL = function(){  //生成URL地址
		return 'http://'+this._IP+':'+this._PORT;
	}
	//获取设备信息
	this.dvrBasicInfo2UI = function(){
		emptyDevSetMenu();
		console.log("-------------------dvrBasicInfo2UI------------------------------");
		
		var This = this;
		var xmlstr = '';
	        xmlstr += '<juan ver="0" squ="fastweb" dir="0">';
	        xmlstr += '<envload type="0" usr="' + this._USR + '" pwd="' + this._PWD + '"/>';
	        xmlstr += '<devinfo name="" model="" hwver="" swver="" reldatetime="" ip="" httpport="" clientport="" rip="" rhttpport="" rclientport="" camcnt="" audcnt="" sensorcnt="" alarmcnt="" />';
	        xmlstr += '</juan>';
			
		dataType='jsonp';  //数据类型
		jsonp='jsoncallback'; // 回调函数
		
		_AJAXget(this.getRequestURL()+'/cgi-bin/gw.cgi?f=j','xml='+xmlstr,'',function(data){
			data2UI(data);
			This._VER = $('#set_content div.dvr_list:visible input[data-UI="swver"]').val();  //软件版本号
			This._CHN = $('#set_content div.dvr_list:visible input[data-UI="camcnt"]').val(); //设备通道数
			console.log(This._CHN+"---通道数---");
		}); 
		   
	
	//获取常规设置信息
	this.dvrGenSet2UI = function(){
		emptyDevSetMenu();
		console.log("-------------------dvrGenSet2UI------------------------------");

		var warp = $('#set_content div.dvr_list:visible');
		    
		
        dataType='jsonp';  //数据类型
		jsonp='jsoncallback'; // 回调函数
		
	    var xmlstr = '';
	    xmlstr += '<juan ver="0" squ="fastweb" dir="0">';
	    xmlstr += '<envload type="0" usr="' + this._USR + '" pwd="' + this._PWD + '">';
	    xmlstr += '<misc datefmt="" keylock="" keybuzzer="" lang="" standard="" dvrid="" hddoverwrite="" alpha="" autoswi="" autoswiinterval="" autoswimode="" />';
	    xmlstr += '</envload>';
	    xmlstr += '</juan>';
       
		_AJAXget(this.getRequestURL()+'/cgi-bin/gw.cgi?f=j','xml='+xmlstr,'',function(data){
			
			data2UI(data);
			
			
		});  

       }
	  	}
	//设置常规信息
	this.dvrGenSetPut = function(){
		console.log("-------------------dvrnetworkInfo2UI------------------------------");
		var warp = $('#set_content div.dvr_list:visible'),
		    str="saveing",
		    oHint = showAJAXHint(str).css('top',warp.height() + 46),
			common_date_format,common_standard,common_autoswiinterval,common_autoswimode,common_autoswimode;
		
	switch(warp.find('input[data-UI="datefmt"]').val())
	{
		case 'YYYY/MM/DD':common_date_format = 0;break;
		case 'MM/DD/YYYY':common_date_format = 1;break;
		case 'DD/MM/YYYY':common_date_format = 2;break;
		default: break;
	}
	var  common_keypad = warp.find('input[data-UI="keylock"]').prop('checked') ?1:0;
	var  common_key_sound = warp.find('input[data-UI="keybuzzer"]').prop('checked') ?1:0;
	switch(warp.find('input[data-UI="standard"]').val())
	{
		case 'PAL':	common_standard = 0;break;
		case 'NTST':common_standard = 1;break;
		default: break;
	}
	var common_over_harddisk = warp.find('input[data-UI="hddoverwrite"]').prop('checked') ?1:0;
	var common_autoswi =  warp.find('input[data-UI="autoswi"]').prop('checked') ?1:0;
	switch( warp.find('input[data-UI="autoswiinterval"]').val())
	{
		case '2s': common_autoswiinterval = 0;break;
		case '3s': common_autoswiinterval = 1;break;
		case '4s': common_autoswiinterval = 2;break;
		case '5s': common_autoswiinterval = 3;break;
		case '8s': common_autoswiinterval = 4;break;
		case '10s':common_autoswiinterval = 5;break;
		default: break;
	}
	switch(warp.find('input[data-UI="autoswimode"]').val())
	{
		
		case '全屏': common_autoswimode =0;break;
		case '屏幕分割': common_autoswimode =1;break;
		default:break;
		
		}
	var xmlstr = '';
	xmlstr += '<juan ver="0" squ="fastweb" dir="0">';
	xmlstr += '<envload type="1" usr="' +this._USR + '" pwd="' + this._PWD + '">';
	xmlstr += '<misc';
	xmlstr += ' datefmt="' + common_date_format + '"';
	xmlstr += ' keylock="' + common_keypad + '"';
	xmlstr += ' keybuzzer="' + common_key_sound + '"';
	xmlstr += ' standard="' + common_standard + '"';
	xmlstr += ' dvrid="' +  warp.find('input[data-UI="dvrid"]').val() + '"';
	xmlstr += ' hddoverwrite="' + common_over_harddisk + '"';
	xmlstr += ' alpha="' + warp.find('input[data-UI="alpha"]').val() + '"';
	xmlstr += ' autoswi="' + common_autoswi + '"';
	xmlstr += ' autoswiinterval="' + common_autoswiinterval + '"';
	xmlstr += ' autoswimode="' +  common_autoswimode + '"';
	xmlstr += ' />';
	xmlstr += '</envload>';
	xmlstr += '</juan>';
	console.log(xmlstr);
	
    dataType='jsonp';  //数据类型
	jsonp='jsoncallback'; // 回调函数
	 
	_AJAXget(this.getRequestURL()+'/cgi-bin/gw.cgi?f=j','xml='+xmlstr,false,'',function(str){
		  if(str=='loading_success')
		     showAJAXHint("save_success").fadeOut(2000);
		   else
		      showAJAXHint(str);
		
		});
		
		}
	
	//获取网络设置信息
	this.dvrnetworkInfo2UI = function(){
		emptyDevSetMenu();
		console.log("-------------------dvrnetworkInfo2UI------------------------------");

		var warp = $('#set_content div.dvr_list:visible');
		
		dataType='jsonp';  //数据类型
		jsonp='jsoncallback'; // 回调函数
		
	    var xmlstr = '';
	    xmlstr += '<juan ver="0" squ="fastweb" dir="0">';
	    xmlstr += '<envload type="0" usr="' + this._USR + '" pwd="' + this._PWD + '">';
	    xmlstr += '<network dhcp="" mac="" ip="" submask="" gateway="" dns="" httpport="" clientport="" enetid="" ddns="" ddnsprovider="" ddnsurl="" ddnsusr="" ddnspwd="" pppoe="" pppoeusr="" pppoepwd="" />';
	    xmlstr += '</envload>';
	    xmlstr += '</juan>';
		
		_AJAXget(this.getRequestURL()+'/cgi-bin/gw.cgi?f=j','xml='+xmlstr,'',function(data){
			data2UI(data);
			},function(){
			//checkbox的控制函数
			dvr_disable('dhcp');
			dvr_disable('ddns');
			dvr_disable('pppoe');	
				});  
		
		}
	 //设置网络信息
	 this.dvrnetworkInfoPut = function(){
		 console.log("-------------------dvrnetworkInfoPut------------------------------");
		 var warp = $('#set_content div.dvr_list:visible'),
		     str="saveing",
		     oHint = showAJAXHint(str).css('top',warp.height() + 46),
			 supplier;
		     
		 dataType='jsonp';  //数据类型
		 jsonp='jsoncallback'; // 回调函数
		 
		 var openDH = warp.find('input[data-UI="dhcp"]').prop('checked') ?1:0;
	     var openDD = warp.find('input[data-UI="ddns"]').prop('checked') ?1:0;
	     var OpenPPPOE = warp.find('input[data-UI="pppoe"]').prop('checked') ?1:0;
		 switch(warp.find('input[data-UI="ddnsprovider"]').val()){
			 case 'dyndns.org':   supplier =0;break;
		     case '3322.org':     supplier =1;break;
			 case 'changeip.com': supplier =2;break;
		     default: break;
			 
			 }
	     var xmlstr = '';
	         xmlstr += '<juan ver="0" squ="fastweb" dir="0">';
	         xmlstr += '<envload type="1" usr="' + this._USR + '" pwd="' + this._PWD + '">';
	         xmlstr += '<network';
	         xmlstr += ' dhcp="' + openDH + '"';
	         xmlstr += ' mac="' + warp.find('input[data-UI="mac"]').val()+ '"';
	         xmlstr += ' ip="' + warp.find('input[data-UI="ip"]').val() + '"';
	         xmlstr += ' submask="' +warp.find('input[data-UI="submask"]').val() + '"';
	         xmlstr += ' gateway="' + warp.find('input[data-UI="gateway"]').val() + '"';
	         xmlstr += ' dns="' + warp.find('input[data-UI="dns"]').val() + '"';
	         xmlstr += ' httpport="' +  warp.find('input[data-UI="clientport"]').val() + '"';
	         xmlstr += ' clientport="' + warp.find('input[data-UI="httpport"]').val() + '"';
	         xmlstr += ' enetid="' +warp.find('input[data-UI="enetid"]').val() + '"';
	         xmlstr += ' ddns="' + openDD + '"';
	         xmlstr += ' ddnsprovider="' + supplier + '"';
	         xmlstr += ' ddnsurl="' + warp.find('input[data-UI="ddnsurl"]').val() + '"';
	         xmlstr += ' ddnsusr="' + warp.find('input[data-UI="ddnsusr"]').val() + '"';
	         xmlstr += ' ddnspwd="' +warp.find('input[data-UI="ddnspwd"]').val()+ '"';
	         xmlstr += ' pppoe="' + OpenPPPOE + '"';
	         xmlstr += ' pppoeusr="' +warp.find('input[data-UI="pppoeusr"]').val() + '"';
	         xmlstr += ' pppoepwd="' + warp.find('input[data-UI="pppoepwd"]').val()+ '"';
	         xmlstr += ' />';
	         xmlstr += '</envload>';
	         xmlstr += '</juan>';
            console.log(xmlstr);
			
		 _AJAXget(this.getRequestURL()+'/cgi-bin/gw.cgi?f=j','xml='+xmlstr,false,'',function(str){
		  if(str=='loading_success'){
		     showAJAXHint("save_success").fadeOut(2000);
		  }else{
		      showAJAXHint(str);
		  }
			
			//qob.OnModifyDeviceEx(); //从IPC的网络设置那边copy过来的，找不到原函数，所以无法确定内部是怎么同步的
			//console.log('--修改端口和地址重新同步设备状态----');
			//areaList2Ui();
			//console.log('--修改端口和地址重新加载设备列表----');
			//已经修改，ipc和dvr都可以重新加载设备信息
			//reInitNowDev();
			//console.log('IP修改成功++1'+This.getRequestURL());
		});
		 
		 }
	//获取编码设置信息
	this.dvrencodeInfo2UI = function(){
		console.log("-------------------dvrencodeInfo2UI------------------------------");
		this._dvrencodeInfo2UI(0);
		}
	this._dvrencodeInfo2UI = function(num){
		emptyDevSetMenu();
		console.log("-------------------_dvrencodeInfo2UI------------------------------");
        var This = this;
		var warp = $('#set_content div.dvr_list:visible');
		
		dataType='jsonp';
		jsonp='jsoncallback';
		
		var xmlstr = '';
	        xmlstr += '<juan ver="0" squ="fastweb" dir="0">';
	        xmlstr += '<envload type="0" usr="' + this._USR + '" pwd="' + this._PWD+ '">';
		    xmlstr += '<encode chn="' + num + '" mode="" fmt="" piclv="" bitmode="" bitvalue="" framerate="" />';
		    xmlstr += '<encodesub chn="' + num + '" mode="" fmt="" piclv="" bitmode="" bitvalue="" framerate="" />';
	        xmlstr += '</envload>';
	        xmlstr += '</juan>';
	   // console.log(xmlstr);
		_AJAXget(this.getRequestURL()+'/cgi-bin/gw.cgi?f=j','xml='+xmlstr,function(){
			 var warpKey='[data-WARP="envload"]';
			 
		    warp.find('ul'+warpKey).html('');
			warp.find('#Encoding_settings div').hide();
			
			for(var j=0;j<This._CHN;j++)
			{   
			     warp.find('ul'+warpKey).append('<li onclick="getencode('+j+')"><input type="text" value="'+(j+1)+'" disabled /></li>');
				 warp.find('#Encoding_settings div').eq(j).show();
			     
				}
			
			},function(data){
			   data2UI(data);
			  warp.find('#Encoding_settings input[type="checkbox"]').eq(data.juan.envload.encode.chn).prop('checked',true);
			   
			},function(){ 
				 warp.find('#Encoding_settings input:checked').prop('disabled',true);
			}); 
		}
		//设置编码信息
	    this.dvrencodeInfoPut = function(){
		     console.log("-------------------dvrencodeInfoPut------------------------------");
		   
		     var warp = $('#set_content div.dvr_list:visible'),
			     object = [],
		         str="saveing",
		         oHint = showAJAXHint(str).css('top',warp.height() + 46),
				 finish = this.checkMultRequests,
			     chk_chn,enc_main_mode,enc_main_format,enc_main_bitvalue,enc_main_framerate,enc_main_piclv,enc_main_bitmode;
				 
		     dataType='jsonp';
		     jsonp='jsoncallback';
			 async=false;
			//var chk_chn =warp.find('input[data-UI="chn"]').val()-1;
			switch(warp.find('td[data-WARP="encode"] input[data-UI="mode"]').val()){
				   case '视频流': enc_main_mode=0;break;
				   case '音视频流': enc_main_mode=1;break;
				   default: break;
				}
	         
	        switch(warp.find('td[data-WARP="encode"] input[data-UI="fmt"]').val())
	        {
		       case 'QCIP': enc_main_format = 0;break;
		       case 'CIF':	enc_main_format = 1;break;
		       case 'HD1':	enc_main_format = 2;break;
		       case 'D1':   enc_main_format = 3;break;
		       default: break;
	        }
	       switch(warp.find('td[data-WARP="encode"] input[data-UI="bitvalue"]').val())
	       {
		      case '64kbps':  enc_main_bitvalue = 0;break;
		      case '128kbps': enc_main_bitvalue = 1;break;
		      case '256kbps': enc_main_bitvalue = 2;break;
		      case '384kbps': enc_main_bitvalue = 3;break;
		      case '512kbps': enc_main_bitvalue = 4;break;
		      case '768kbps': enc_main_bitvalue = 5;break;
		      case '1Mbps':   enc_main_bitvalue = 6;break;
		      case '1.5Mbps': enc_main_bitvalue = 7;break;
		      case '2Mbps':   enc_main_bitvalue = 8;break;
		      default: break;
	     }
	     switch(warp.find('td[data-WARP="encode"] input[data-UI="framerate"]').val())
	    {
		     case '1FPS': enc_main_framerate = 0;break;
		     case '2FPS': enc_main_framerate = 1;break;
		     case '3FPS': enc_main_framerate = 2;break;
		     case '5FPS': enc_main_framerate = 3;break;
		     case '8FPS': enc_main_framerate = 4;break;
		     case '10FPS': enc_main_framerate = 5;break;
		     case '12FPS': enc_main_framerate = 6;break;
		     case '15FPS': enc_main_framerate = 7;break;
		     case '20FPS': enc_main_framerate = 8;break;
		     case '25FPS': enc_main_framerate = 9;break;
		     default: break;
		}
		switch(warp.find('td[data-WARP="encode"] input[data-UI="piclv"]').val()){
				   case '最高':  enc_main_piclv=0;break;
				   case '高':   enc_main_piclv=1;break;
				   case '适中': enc_main_piclv=2;break;
				   case '低':   enc_main_piclv=3;break;
				   case '最低': enc_main_piclv=4;break;
				   default: break;
				}
		switch(warp.find('td[data-WARP="encode"] input[data-UI="bitmode"]').val()){
				   case '固定码率':  enc_main_bitmode=0;break;
				   case '可变码率':  enc_main_bitmode=1;break;
				   case '适中码率': enc_main_bitmode=2;break;
				   default: break;
				}		
	   //sub_stream
	    var enc_sub_format,enc_sub_bitvalue,enc_sub_framerate,enc_sub_piclv,enc_sub_bitmode,enc_sub_mode;
	    switch(warp.find('td[data-WARP="encodesub"] input[data-UI="fmt"]').val())
	    {
		     case 'QCIP': enc_sub_format = 0;break;
		     case 'CIF':  enc_sub_format = 1;break;
		     default: break;
	    }
	    switch(warp.find('td[data-WARP="encodesub"] input[data-UI="bitvalue"]').val())
	   {
		    case '64kbps': enc_sub_bitvalue = 0;break;
		    case '128kbps':	enc_sub_bitvalue = 1;break;
		    case '256kbps':	enc_sub_bitvalue = 2;break;
		    case '384kbps':	enc_sub_bitvalue = 3;break;
		    default: break;
	   }
	   switch(warp.find('td[data-WARP="encodesub"] input[data-UI="framerate"]').val())
	   {
		   case '1FPS': enc_sub_framerate = 0;break;
		   case '2FPS': enc_sub_framerate = 1;break;
		   case '3FPS': enc_sub_framerate = 2;break;
		   case '5FPS': enc_sub_framerate = 3;break;
		   case '8FPS': enc_sub_framerate = 4;break;
		   case '10FPS': enc_sub_framerate = 5;break;
		   case '12FPS': enc_sub_framerate = 6;break;
		   default: break;
	   }
	   switch(warp.find('td[data-WARP="encodesub"] input[data-UI="piclv"]').val()){
				   case '最高':  enc_sub_piclv=0;break;
				   case '高':   enc_sub_piclv=1;break;
				   case '适中': enc_sub_piclv=2;break;
				   case '低':   enc_sub_piclv=3;break;
				   case '最低': enc_sub_piclv=4;break;
				   default: break;
				}
	  switch(warp.find('td[data-WARP="encodesub"] input[data-UI="bitmode"]').val()){
				   case '固定码率':  enc_sub_bitmode=0;break;
				   case '可变码率':  enc_sub_bitmode=1;break;
				   case '适中码率':  enc_sub_bitmode=2;break;
				   default: break;
				}
	switch(warp.find('td[data-WARP="encodesub"] input[data-UI="mode"]').val()){
				   case '视频流': enc_sub_mode=0;break;
				  // case '音视频流': enc_main_mode=1;break;
				   default: break;
				}		
	  var xmlstr = '';
	      xmlstr += '<juan ver="0" squ="fastweb" dir="0">';
	      xmlstr += '<envload type="1" usr="' + this._USR + '" pwd="' + this._PWD+ '">';
	      xmlstr += '<encode';
	      xmlstr += ' chn="' +  chk_chn + '"';
	      xmlstr += ' mode="' + enc_main_mode + '"';
	      xmlstr += ' fmt="' +  enc_main_format + '"';
	      xmlstr += ' piclv="' + enc_main_piclv + '"';
	      xmlstr += ' bitmode="' + enc_main_bitmode + '"';
	      xmlstr += ' bitvalue="' + enc_main_bitvalue + '"';
	      xmlstr += ' framerate="' + enc_main_framerate + '"';
	      xmlstr += ' />';
	      xmlstr += '<encodesub';
	      xmlstr += ' chn="' + chk_chn + '"';
	      xmlstr += ' mode="' + enc_sub_mode + '"';
	      xmlstr += ' fmt="' + enc_sub_format + '"';
	      xmlstr += ' piclv="' +enc_sub_piclv + '"';
	      xmlstr += ' bitmode="' + enc_sub_bitmode + '"';
	      xmlstr += ' bitvalue="' + enc_sub_bitvalue + '"';
	      xmlstr += ' framerate="' + enc_sub_framerate + '"';
	      xmlstr += ' />';
	      xmlstr += '</envload>';
	      xmlstr += '</juan>';
	 console.log(xmlstr);
	      
	      
	     warp.find('#Encoding_settings input[type="checkbox"]').each(function(){
          if($(this).prop('checked')==true) {
               object.push($(this).siblings("span").html());
             }
	     console.log(object);
          });
	    	_Request=new Array(object.length);
			
		for(var i in object){	
		    chk_chn = object[i];
		 _AJAXget(this.getRequestURL()+'/cgi-bin/gw.cgi?f=j','xml='+xmlstr,false,'',finish);
		
		}
		async=true;
		}
		
		
	   //获取录像设置信息
	   this.dvrVideoInfo2UI = function(){
		    console.log("-------------------dvrVideoInfo2UI------------------------------");
		    this._dvrVideoInfo2UI(0,0);
		   }
	   this._dvrVideoInfo2UI = function(num,week){
		   emptyDevSetMenu();
		   console.log("-------------------_dvrVideoInfo2UI------------------------------");
		   
		   var This = this;
		   var warp = $('#set_content div.dvr_list:visible');
		  
		   dataType='jsonp';
		   jsonp='jsoncallback';
		   
		   var xmlstr = '';
	           xmlstr += '<juan ver="0" squ="fastweb" dir="0">';
	           xmlstr += '<envload type="0" usr="' + this._USR + '" pwd="' + this._PWD+ '">';
	      for(var i = 0; i < 4; i ++)
	      {
		       xmlstr += '<record chn="' + num + '" weekday="' + week + '" index="' + i + '" begin="" end="" types="" />';
	      }
	           xmlstr += '</envload>';
	           xmlstr += '</juan>';
			   
		  _AJAXget(this.getRequestURL()+'/cgi-bin/gw.cgi?f=j','xml='+xmlstr,function(){
			 var warpKey='[data-WARP="envload"]';
			   warp.find('input[data-UI="chn1"]').val(num+1);
		     
			$('#dvr_record_week_sel').find('li').remove();//清除星期
			warp.find('ul'+warpKey).html('');
			
			warp.find('#Video_settings div').hide();
			
			for(var j=0;j<This._CHN;j++)
			{   
			     warp.find('ul'+warpKey).append('<li onclick="getRecord('+j+','+0+')"><input value="'+(j+1)+'" disabled /></li>');
				 warp.find('#Video_settings div').eq(j).show();
				
				}
			
			},function(data){
				data2UI(data);
			    warp.find('#Video_settings input[type="checkbox"]').eq(num).prop('checked',true);
			   
			},function(){ 
				 warp.find('#Video_settings input:checked').prop('disabled',true);
			});  
		   
		   
		   } 
	   
	  //获取屏幕设置信息
	   this.dvrScreenInfo2UI = function(){
		   console.log("-------------------dvrScreenInfo2UI------------------------------");
		   this._dvrScreenInfo2UI(0);
		   }
	   this._dvrScreenInfo2UI = function(num){
		 emptyDevSetMenu();
		console.log("---num---"+num);
		dataType='jsonp';
		jsonp='jsoncallback';
		
		var This =this;
		var warp = $('#set_content div.dvr_list:visible');
		
		var xmlstr = '';
	        xmlstr += '<juan ver="0" squ="fastweb" dir="0" enc="1">';
	        xmlstr += '<envload type="0" usr="' + this._USR + '" pwd="' + this._PWD + '">';
		    xmlstr += '<screen chn="' + num + '" title=""/>';
	        xmlstr += '</envload>';
	        xmlstr += '</juan>';
		  // console.log(xmlstr);
		
		_AJAXget(this.getRequestURL()+'/cgi-bin/gw.cgi?f=j','xml='+xmlstr,function(){
			     
				  var warpKey='[data-WARP="envload"]';
			       warp.find('ul'+warpKey).html('');
				   
					for(var k =0; k<This._CHN;k++){
						warp.find('ul'+warpKey).append('<li onclick="getTitle('+k+')"><input type="text" value="'+(k+1)+'" disabled /></li>');	
					}
			
			},function(data){
			 data2UI(data);
			});
		
		
		} 
     //设置屏幕信息
	   this.dvrScreenInfoPut = function(){
		   console.log("-------------------dvrScreenInfoPut------------------------------");
		    var warp = $('#set_content div.dvr_list:visible'),
		     str="saveing",
		     oHint = showAJAXHint(str).css('top',warp.height() + 46);
			 
		     
		 dataType='jsonp';  //数据类型
		 jsonp='jsoncallback'; // 回调函数
		 
		 var chk_chn =warp.find('input[data-UI="chn"]').val() -1;
	     var xmlstr = '';
	         xmlstr += '<juan ver="0" squ="fastweb" dir="0" enc="1">';
	         xmlstr += '<envload type="1" usr="' + this._USR + '" pwd="' + this._PWD + '">';
	         xmlstr += '<screen';
	         xmlstr += ' chn="' + chk_chn + '"';
	         xmlstr += ' title="' + warp.find('input[data-UI="title"]').val() + '"';
	         xmlstr += ' />';
	         xmlstr += '</envload>';
	         xmlstr += '</juan>';
         console.log(xmlstr);
			
		 _AJAXget(this.getRequestURL()+'/cgi-bin/gw.cgi?f=j','xml='+xmlstr,false,'',function(str){
		  if(str=='loading_success')
		     showAJAXHint("save_success").fadeOut(2000);
		   else
		      showAJAXHint(str);
		
		});
		   
		   
		   }
	
	//获取云台设置信息
	this.dvrPTZInfo2UI = function(){
		console.log("-------------------dvrPTZInfo2UI------------------------------");
		this._dvrPTZInfo2UI(0);
		
		}
	this._dvrPTZInfo2UI = function(num){
		emptyDevSetMenu();
		
		dataType='jsonp';  //数据类型
		jsonp='jsoncallback'; // 回调函数
		
		var This = this;
		var warp = $('#set_content div.dvr_list:visible');
		
		var xmlstr = '';
	        xmlstr += '<juan ver="0" squ="fastweb" dir="0">';
	        xmlstr += '<envload type="0" usr="' + this._USR + '" pwd="' + this._PWD + '">';
	        xmlstr += '<ptz chn="' + num + '" id="" protocal="" baudrate="" databit="" stopbit="" parity=""/>';
	        xmlstr += '</envload>';
	        xmlstr += '</juan>';
		
		_AJAXget(this.getRequestURL()+'/cgi-bin/gw.cgi?f=j','xml='+xmlstr,function(){
			
			
			var warpKey='[data-WARP="envload"]';
			
			 warp.find('ul'+warpKey).html('');
			 warp.find('#PTZ_settings div').hide();
			 
			for(var j=0;j<This._CHN;j++)
			{    
			     warp.find('ul'+warpKey).append('<li onclick="getPTZ('+j+')"><input type="text" value="'+(j+1)+'" disabled /></li>');
				 
				 warp.find('#PTZ_settings div').eq(j).show();
				
				}	
			},function(data){	     
				 data2UI(data);
			     warp.find('#PTZ_settings input:checkbox').eq(data.juan.envload.ptz.chn).prop('checked',true);
			},function(){ 
				 warp.find('#PTZ_settings input:checked').prop('disabled',true);
			});
		}
	//获取云台设置信息
	this.dvrPTZInfoPut = function(){
		
		}
	//获取视频检验设置信息
	this.dvrVideoCkeck2UI = function(){
		console.log("-------------------dvrVideoCkeck2UI------------------------------");
		this._dvrVideoCkeck2UI(0);
		}
	this._dvrVideoCkeck2UI = function(num){
		emptyDevSetMenu();
		
		
		dataType='jsonp';  //数据类型
		jsonp='jsoncallback'; // 回调函数
		
		var This = this,chn_num=num;
		var warp = $('#set_content div.dvr_list:visible');
		
		var xmlstr = '';
	        xmlstr += '<juan ver="0" squ="fastweb" dir="0">';
	        xmlstr += '<envload type="0" usr="' + this._USR + '" pwd="' + this._PWD + '">';
		    xmlstr += '<detection chn="' + num + '" sens="" mdalarmduration="" mdalarm="" mdbuzzer="" vlalarmduration="" vlalarm="" vlbuzzer="" />';
	        xmlstr += '</envload>';
	        xmlstr += '</juan>';
		_AJAXget(this.getRequestURL()+'/cgi-bin/gw.cgi?f=j','xml='+xmlstr,function(){
			var warpKey='[data-WARP="envload"]';
			
			 warp.find('ul'+warpKey).html('');
			 warp.find('#Video_detection_settings div').hide();
			for(var j=0;j<This._CHN;j++)
			{    
			     warp.find('ul'+warpKey).append('<li onclick="getVideo('+j+')"><input type="text" value="'+(j+1)+'" disabled /></li>');
				 warp.find('#Video_detection_settings div').eq(j).show();			 
				}
			
			},function(data){
			     
				 data2UI(data);
			     warp.find('#Video_detection_settings input[type="checkbox"]').eq(data.juan.envload.detection.chn).prop('checked',true);
			},function(){ 
				 warp.find('#Video_detection_settings input:checked').prop('disabled',true);
			});
		
		}
		 
	//获取报警设置信息
	this.dvrAlarmInfo2UI = function(){
		
		//emptyDevSetMenu();
		console.log("-------------------dvrAlarmInfo2UI------------------------------");
		this._dvrAlarmInfo2UI(0);
	}
	this._dvrAlarmInfo2UI = function(num){ 
	
	  console.log("-------------------_dvrAlarmInfo2UI------------------------------");
	  
		dataType='jsonp';  //数据类型
		jsonp='jsoncallback'; // 回调函数
		
		var This = this;
		var warp = $('#set_content div.dvr_list:visible');
		
		var xmlstr = '';
	        xmlstr += '<juan ver="0" squ="fastweb" dir="0">';
	        xmlstr += '<envload type="0" usr="' + this._USR + '" pwd="' + this._PWD + '">';
		    xmlstr += '<sensor chn="' + num + '" mode="" alarmduration="" alarm="" buzzer="" />';
	        xmlstr += '</envload>';
	        xmlstr += '</juan>';
		
		_AJAXget(this.getRequestURL()+'/cgi-bin/gw.cgi?f=j','xml='+xmlstr,function(){
			var warpKey='[data-WARP="envload"]';
			
			 warp.find('ul'+warpKey).html('');
			 
			 warp.find('#Alarm_settings div').hide();
			for(var j=0;j<This._CHN;j++)
			{
				 warp.find('ul'+warpKey).append('<li onclick="getAlarm('+j+')"><input type="text" value="'+(j+1)+'" disabled /></li>');
				 warp.find('#Alarm_settings div').eq(j).show();
				
				}
	            
				
			 
			},function(data){
			     
				data2UI(data);
			    warp.find('#Alarm_settings input[type="checkbox"]').eq(data.juan.envload.sensor.chn).prop('checked',true);
			},function(){ 
				 warp.find('#Alarm_settings input:checked').prop('disabled',true);
			});
		
	
		}
		
	//多次请求，提示信息在所有请求都成功之后才能显示成功
     this.checkMultRequests = function(str){
		 
	   console.log(str);
	   console.log(_Request.length);
	   console.log(_Request);
	   
		RequesePush(_Request,str);
		
		var l = _Request.length -1;

		if(!_Request[l]) return; //当ajax请求时，数组的最后一个数据仍为undefined，则退出函数，继续循环执行请求，直到所以请求都完成
			

		for(var i=0;i<_Request.length;i++){
			
			if(_Request[i]=='loading_success' || _Request[i]=='save_success' ){
				showAJAXHint(_Request[i]).fadeOut(2000);
			}else{
				showAJAXHint(_Request[i]);
				}

		}
		function RequesePush(arr,str){
			for(var i=0;i<arr.length;i++){
				if(!arr[i]){ //空数组的类型为undefined，！undefined即为true
					arr[i]=str;
					return ; 
				}
			}
		}
		 
		 
		 }
	return this;
	}