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
			//console.log(This._CHN+"---通道数---");
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
       
		_AJAXget(this.getRequestURL()+'/cgi-bin/gw.cgi?f=j','xml='+xmlstr,'',function(data){ data2UI(data);});  

       }
	  	}
	//设置常规信息
	this.dvrGenSetPut = function(){
		console.log("-------------------dvrnetworkInfo2UI------------------------------");
		var warp = $('#set_content div.dvr_list:visible'),
		    str="saveing",
		    oHint = showAJAXHint(str).css('top',warp.height() + 46),
			getCheckbox = this.getCheckbox,
			getSelect = this.getSelect,
			getValue = this.getValue;
	
	
	  var xmlstr = '';
	  xmlstr += '<juan ver="0" squ="fastweb" dir="0">';
	  xmlstr += '<envload type="1" usr="' +this._USR + '" pwd="' + this._PWD + '">';
	  xmlstr += '<misc';
	  xmlstr += ' datefmt="' + getSelect("datefmt") + '"';
	  xmlstr += ' keylock="' + getCheckbox("keylock"); + '"';
	  xmlstr += ' keybuzzer="' + getCheckbox("keybuzzer") + '"';
	  xmlstr += ' standard="' + getSelect("standard") + '"';
	  xmlstr += ' dvrid="' +  getValue("dvrid") + '"';
	  xmlstr += ' hddoverwrite="' + getCheckbox("hddoverwrite") + '"';
	  xmlstr += ' alpha="' + getSelect("alpha") + '"';
	  xmlstr += ' autoswi="' + getCheckbox("autoswi") + '"';
	  xmlstr += ' autoswiinterval="' + getSelect("autoswiinterval") + '"';
	  xmlstr += ' autoswimode="' + getSelect("autoswimode") + '"';
	  xmlstr += ' />';
	  xmlstr += '</envload>';
	  xmlstr += '</juan>';
	  //console.log(xmlstr);
	  
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
			dvr_disable('dhcp',true,false);
			dvr_disable('ddns',false,true);
			dvr_disable('pppoe',false,true);	
				});  
		
		}
		
	 //设置网络信息
	 this.dvrnetworkInfoPut = function(){
		 console.log("-------------------dvrnetworkInfoPut------------------------------");
		 var warp = $('#set_content div.dvr_list:visible'),
		     str="saveing",
		     oHint = showAJAXHint(str).css('top',warp.height() + 46),
			 getCheckbox = this.getCheckbox,
			 getSelect = this.getSelect,
			 getValue = this.getValue;
			 
		     
		 dataType='jsonp';  //数据类型
		 jsonp='jsoncallback'; // 回调函数
		
	     var xmlstr = '';
	         xmlstr += '<juan ver="0" squ="fastweb" dir="0">';
	         xmlstr += '<envload type="1" usr="' + this._USR + '" pwd="' + this._PWD + '">';
	         xmlstr += '<network';
	         xmlstr += ' dhcp="' + getCheckbox("dhcp") + '"';
	         xmlstr += ' mac="' + getValue("mac")+ '"';
	         xmlstr += ' ip="' + getValue("ip") + '"';
	         xmlstr += ' submask="' +getValue("submask") + '"';
	         xmlstr += ' gateway="' + getValue("gateway") + '"';
	         xmlstr += ' dns="' + getValue("dns") + '"';
	         xmlstr += ' httpport="' + getValue("httpport") + '"';
	         xmlstr += ' clientport="' + getValue("clientport") + '"';
	         xmlstr += ' enetid="' +getValue("enetid") + '"';
	         xmlstr += ' ddns="' + getCheckbox("ddns") + '"';
	         xmlstr += ' ddnsprovider="' + getSelect("ddnsprovider") + '"';
	         xmlstr += ' ddnsurl="' + getValue("ddnsurl") + '"';
	         xmlstr += ' ddnsusr="' + getValue("ddnsusr") + '"';
	         xmlstr += ' ddnspwd="' +getValue("ddnspwd")+ '"';
	         xmlstr += ' pppoe="' + getCheckbox("pppoe") + '"';
	         xmlstr += ' pppoeusr="' +getValue("pppoeusr") + '"';
	         xmlstr += ' pppoepwd="' + getValue("pppoepwd")+ '"';
	         xmlstr += ' />';
	         xmlstr += '</envload>';
	         xmlstr += '</juan>';
           // console.log(xmlstr);
			
		 _AJAXget(this.getRequestURL()+'/cgi-bin/gw.cgi?f=j','xml='+xmlstr,false,'',function(str){
		  if(str=='loading_success'){
		     showAJAXHint("save_success").fadeOut(2000);
		  }else{
		      showAJAXHint(str);
		  }
			//console.log('--修改端口和地址重新同步设备状态----');
	        $("#address_ID").val($("#dvr_ip_addr").attr('data')); 
			$("#port_ID").val($("#dvr_web_port").attr("data")); 
			$("#http_ID").val($("#dvr_web_port").attr("data")); 
		    $("#ModifyDevice_ok").click;
			//console.log('--修改端口和地址重新加载设备列表----');
			reInitNowDev();
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
		
		this.Adjust(This._CHN,$('#Encoding_settings'));
		
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
			     warp.find('ul'+warpKey).append('<li onclick="getDataByChn('+j+');"><input type="text" value="'+(j+1)+'" disabled /></li>');
				 warp.find('#Encoding_settings div').eq(j).show();
			     
				}
		
			},function(data){
			  data2UI(data);
			  warp.find('#Encoding_settings input[type="checkbox"]').prop('checked',false).eq(data.juan.envload.encode.chn).prop('checked',true);
			   
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
				 getCheckbox = this.getCheckbox,
			     getSelect = this.getSelect,
			     getValue = this.getValue,
				 chn_val = parseInt(getValue('chn'));
			    
				 
		     dataType='jsonp';
		     jsonp='jsoncallback';
			 async=false;
	
	     warp.find('#Encoding_settings input[type="checkbox"]:visible').each(function(){
          if($(this).prop('checked')==true) {
               object.push($(this).siblings("span").html());
             }
	     console.log(object);
          });
	    	_Request=new Array(object.length);
			
		for(var i in object){	
		
		  chk_chn = object[i]-1;
			
		  var xmlstr = '';
	      xmlstr += '<juan ver="0" squ="fastweb" dir="0">';
	      xmlstr += '<envload type="1" usr="' + this._USR + '" pwd="' + this._PWD+ '">';
	      xmlstr += '<encode';
	      xmlstr += ' chn="' +  chk_chn + '"';
	      xmlstr += ' mode="' + getSelect("mode","encode") + '"';
	      xmlstr += ' fmt="' + getSelect("fmt","encode") + '"';
	      xmlstr += ' piclv="' + getSelect("piclv","encode") + '"';
	      xmlstr += ' bitmode="' + getSelect("bitmode","encode") + '"';
	      xmlstr += ' bitvalue="' + getSelect("bitvalue","encode") + '"';
	      xmlstr += ' framerate="' + getSelect("framerate","encode") + '"';
	      xmlstr += ' />';
	      xmlstr += '<encodesub';
	      xmlstr += ' chn="' + chk_chn + '"';
	      xmlstr += ' mode="' + getSelect("mode","encodesub") + '"';
	      xmlstr += ' fmt="' +  getSelect("fmt","encodesub") + '"';
	      xmlstr += ' piclv="' + getSelect("piclv","encodesub") + '"';
	      xmlstr += ' bitmode="' + getSelect("framerate","encodesub") + '"';
	      xmlstr += ' bitvalue="' + getSelect("bitmode","encodesub") + '"';
	      xmlstr += ' framerate="' + getSelect("framerate","encodesub") + '"';
	      xmlstr += ' />';
	      xmlstr += '</envload>';
	      xmlstr += '</juan>';
		  
	   //console.log(xmlstr);	
	  
		 _AJAXget(this.getRequestURL()+'/cgi-bin/gw.cgi?f=j','xml='+xmlstr,false,'',finish);
		
		}
		 $('#Encoding_settings input:checkbox').prop('checked',false).eq(chn_val-1).prop('checked',true);
		  $('#Encoding_settings_SelectAll').prop('checked',false);
		async=true;
		}

	   //获取录像设置信息
	   this.dvrVideoInfo2UI = function(){
		    console.log("-------------------dvrVideoInfo2UI------------------------------");
		    this._dvrVideoInfo2UI(0,0);
		   }
	   this._dvrVideoInfo2UI = function(num,week){
		   num =parseInt(num,10);
		   emptyDevSetMenu();
		   console.log("-------------------_dvrVideoInfo2UI------------------------------");
		   
		   var This = this;
		   
		   this.Adjust(This._CHN,$('#Video_settings'));
			
		   var warp = $('#set_content div.dvr_list:visible');
		  
		   dataType='jsonp';
		   jsonp='jsoncallback';
		   
		   var xmlstr = '';
	           xmlstr += '<juan ver="0" squ="fastweb" dir="0">';
	           xmlstr += '<envload type="0" usr="' + this._USR + '" pwd="' + this._PWD+ '">';
			   for(var i=0;i<4;i++){
		           xmlstr += '<record chn="' + num + '" weekday="' + week + '" index="' + i + '" begin="" end="" types="" />';
			   }
	           xmlstr += '</envload>';
	           xmlstr += '</juan>';
			  console.log(xmlstr); 
		  _AJAXget(this.getRequestURL()+'/cgi-bin/gw.cgi?f=j','xml='+xmlstr,function(){
			 var warpKey='[data-WARP="envload"]';
			   warp.find('input[data-UI="chn1"]').val(num+1);	
		     
			$('#dvr_record_week_sel').find('li').remove();//清除星期
			warp.find('ul'+warpKey).html('');
			
			warp.find('#Video_settings div').hide();
			
			for(var j=0;j<This._CHN;j++)
			{   
			     warp.find('ul'+warpKey).append('<li onclick="DataByChn($(this))" data="'+j+'"><input value="'+(j+1)+'" disabled /></li>');
				 warp.find('#Video_settings div').eq(j).show();
				
				}
			
			},function(data){
				data2UI(data);
				var arr = data.juan.envload.record;
				for(var i =0;i<4;i++){
					 var start =This.timeZero(arr[i].begin);
					 var end =This.timeZero(arr[i].end);
					 warp.find(".schedle_id").eq(i).find("input:text").eq(0).val(start);
					 warp.find(".schedle_id").eq(i).find("input:text").eq(0).attr("data",start);
					 warp.find(".schedle_id").eq(i).find("input:text").eq(1).val(end);
					 warp.find(".schedle_id").eq(i).find("input:text").eq(1).attr("data",end);
	
					 
					var types = parseInt(arr[i].types);
					for(var j = 0; j < 3; j++)
					{
						if(types == 0)
						{
							warp.find(".schedle_id").eq(i).find('td input:checkbox').eq(j).prop("checked",false);
						}else{
						 	
						   warp.find(".schedle_id").eq(i).find('td input:checkbox').eq(j).prop("checked",(types & (1 << j)));
						 
						}
					}			
			   }
			    warp.find('#Video_settings input[type="checkbox"]').prop('checked',false).eq(num).prop('checked',true);
			   
			},function(){ 
				 warp.find('#Video_settings input:checked').prop('disabled',true);
			});  
		   } 
	   //设置录像设置信息
	   this.dvrVideoInfoPut = function(){
		    console.log("-------------------dvrVideoInfoPut------------------------------");
		    
			 var warp = $('#set_content div.dvr_list:visible'),
			     object = [],
		         str="saveing",
		         oHint = showAJAXHint(str).css('top',warp.height() + 46),
				 finish = this.checkMultRequests,
				 getCheckbox = this.getCheckbox,
			     getSelect = this.getSelect,
			     getValue = this.getValue,
				 chn_val = parseInt(getValue('chn1'));
			    
				 
		     dataType='jsonp';
		     jsonp='jsoncallback';
			 async=false;
	
	     warp.find('#Video_settings input[type="checkbox"]:visible').each(function(){
          if($(this).prop('checked')==true) {
               object.push($(this).siblings("span").html());
             }
	     console.log(object);
          });
	    	_Request=new Array(object.length);
			
		for(var i in object){	
		
		  chk_chn = object[i]-1;
			
			var xmlstr = '';
			xmlstr += '<juan ver="0" squ="fastweb" dir="0">';
			 xmlstr += '<envload type="1" usr="' + this._USR + '" pwd="' + this._PWD+ '">';
			for(var i = 0; i < 4; i++)
			{
				xmlstr += '<record';
				xmlstr += ' chn="' + chk_chn + '"';
				xmlstr += ' weekday="' + getSelect("weekday") + '"';
				xmlstr += ' index="' + i + '"';
				xmlstr += ' begin="' +warp.find(".schedle_id").eq(i).find("input:visible").eq(0).val() + '"';
				xmlstr += ' end="' +  warp.find(".schedle_id").eq(i).find("input:visible").eq(1).val() + '"';
				var types = 0;
				for(var j = 0; j < 3; j++)
				{
					if( warp.find(".schedle_id").eq(i).next().find('input:checkbox').eq(j).prop("checked"))
					{
						types += Math.pow(2, j);
					}
				}
				xmlstr += ' types="' + types + '"';
				xmlstr += ' />';
			}
		
			xmlstr += '</envload>';
			xmlstr += '</juan>';
			//console.log(xmlstr);
			
			 _AJAXget(this.getRequestURL()+'/cgi-bin/gw.cgi?f=j','xml='+xmlstr,false,'',finish);
		
		}
		 $('#Video_settings input:checkbox').prop('checked',false).eq(chn_val-1).prop('checked',true);
		 $('#Video_settings_SelectAll').prop('checked',false);
		async=true;	  
				  
				  
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
			     
				  var warpKey ='[data-WARP="envload"]';
				          
			           warp.find('ul'+warpKey).html('');
				   
					for(var k =0; k<This._CHN;k++){
						warp.find('ul'+warpKey).append('<li onclick="getDataByChn('+k+');" ><input type="text" value="'+(k+1)+'" disabled /></li>');	
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
		     oHint = showAJAXHint(str).css('top',warp.height() + 46),
			 getValue = this.getValue;
			 
		     
		 dataType='jsonp';  //数据类型
		 jsonp='jsoncallback'; // 回调函数
		 
		 var chk_chn = getValue("chn") -1;
	     var xmlstr = '';
	         xmlstr += '<juan ver="0" squ="fastweb" dir="0" enc="1">';
	         xmlstr += '<envload type="1" usr="' + this._USR + '" pwd="' + this._PWD + '">';
	         xmlstr += '<screen';
	         xmlstr += ' chn="' + chk_chn + '"';
	         xmlstr += ' title="' + getValue("title") + '"';
	         xmlstr += ' />';
	         xmlstr += '</envload>';
	         xmlstr += '</juan>';
        // console.log(xmlstr);
			
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
		this.Adjust(This._CHN,$('#PTZ_settings'));
		var warp = $('#set_content div.dvr_list:visible');
		
		var xmlstr = '';
	        xmlstr += '<juan ver="0" squ="fastweb" dir="0">';
	        xmlstr += '<envload type="0" usr="' + this._USR + '" pwd="' + this._PWD + '">';
	        xmlstr += '<ptz chn="' + num + '" id="" protocal="" baudrate="" databit="" stopbit="" parity=""/>';
	        xmlstr += '</envload>';
	        xmlstr += '</juan>';
		    console.log(xmlstr);
		_AJAXget(this.getRequestURL()+'/cgi-bin/gw.cgi?f=j','xml='+xmlstr,function(){
			
			
			var warpKey='[data-WARP="envload"]';
			
			 warp.find('ul'+warpKey).html('');
			 warp.find('#PTZ_settings div').hide();
			 
			for(var j=0;j<This._CHN;j++)
			{    
			     warp.find('ul'+warpKey).append('<li onclick="getDataByChn('+j+')"><input type="text" value="'+(j+1)+'" disabled /></li>');
				 
				 warp.find('#PTZ_settings div').eq(j).show();
				
				}	
			
			},function(data){	     
				 data2UI(data);
			     warp.find('#PTZ_settings input:checkbox').prop('checked',false).eq(data.juan.envload.ptz.chn).prop('checked',true);
			},function(){ 
				 warp.find('#PTZ_settings input:checked').prop('disabled',true);
			});
		}
	//设置云台设置信息
	this.dvrPTZInfoPut = function(){
	    console.log("-------------------dvrPTZInfoPut------------------------------");
		var  warp = $('#set_content div.dvr_list:visible'),
			 object = [],
			 str="saveing",
			 oHint = showAJAXHint(str).css('top',warp.height() + 46),
			 finish = this.checkMultRequests,
			 getCheckbox = this.getCheckbox,
			 getSelect = this.getSelect,
			 getValue = this.getValue,
			 chn_val = parseInt(getValue('chn'));
		
	    dataType='jsonp';  //数据类型
		jsonp='jsoncallback'; // 回调函数
		async=false;
		
		warp.find('#PTZ_settings input[type="checkbox"]:visible').each(function(){
          if($(this).prop('checked')==true) {
               object.push($(this).siblings("span").html());
             }
	     console.log(object);
          });
	    	_Request=new Array(object.length);
			
		for(var i in object){	
		
		  chk_chn = object[i]-1;
		
		var xmlstr = '';
		xmlstr += '<juan ver="0" squ="fastweb" dir="0">';
		xmlstr += '<envload type="1" usr="' + this._USR + '" pwd="' + this._PWD + '">';
		xmlstr += '<ptz';
		xmlstr += ' chn="' + chk_chn + '"';
		xmlstr += ' id="' + getValue("id") + '"';
		xmlstr += ' protocal="' + getSelect("protocal") + '"';
		xmlstr += ' baudrate="' + getSelect("baudrate") + '"';
		xmlstr += ' />';
		xmlstr += '</envload>';
		xmlstr += '</juan>';
		//console.log(xmlstr);
		
		 _AJAXget(this.getRequestURL()+'/cgi-bin/gw.cgi?f=j','xml='+xmlstr,false,'',finish);
		}
		 $('#PTZ_settings input:checkbox:visible').prop('checked',false).eq(chn_val-1).prop('checked',true);
		 $('#PTZ_settings_SelectAll').prop('checked',false);
		async=true;
		}
	//获取视频检验设置信息
	this.dvrVideoCkeck2UI = function(){
		console.log("-------------------dvrVideoCkeck2UI------------------------------");
		this._dvrVideoCheckInfo2UI(0);
		}
	this._dvrVideoCheckInfo2UI = function(num){
		emptyDevSetMenu();
		
		
		dataType='jsonp';  //数据类型
		jsonp='jsoncallback'; // 回调函数
		
		var This = this,chn_num=num;
		this.Adjust(This._CHN,$('#Video_detection_settings'));
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
			     warp.find('ul'+warpKey).append('<li onclick="getDataByChn('+j+');"><input type="text" value="'+(j+1)+'" disabled /></li>');
				 warp.find('#Video_detection_settings div').eq(j).show();			 
				}
			
			},function(data){
			     
				 data2UI(data);
			     warp.find('#Video_detection_settings input[type="checkbox"]').prop('checked',false).eq(data.juan.envload.detection.chn).prop('checked',true);
			},function(){ 
				 warp.find('#Video_detection_settings input:checked').prop('disabled',true);
			});
		
		}
	//设置视频检验设置信息
	this.dvrVideoCkeckPut = function(){
		console.log("-------------------dvrVideoCkeckPut------------------------------");
		var  warp = $('#set_content div.dvr_list:visible'),
			 object = [],
			 str="saveing",
			 oHint = showAJAXHint(str).css('top',warp.height() + 46),
			 finish = this.checkMultRequests,
			 getCheckbox = this.getCheckbox,
			 getSelect = this.getSelect,
			 getValue = this.getValue,
			 chn_val = parseInt(getValue('chn'));
			 
		dataType='jsonp';  //数据类型
		jsonp='jsoncallback'; // 回调函数
		async=false;
	  
	   warp.find('#Video_detection_settings input[type="checkbox"]:visible').each(function(){
		if($(this).prop('checked')==true) {
			 object.push($(this).siblings("span").html());
		   }
	   console.log(object);
		});
		  _Request=new Array(object.length);
		  
	  for(var i in object){	
	  
		chk_chn = object[i]-1;
		
		var xmlstr = '';
			xmlstr += '<juan ver="0" squ="fastweb" dir="0">';
			xmlstr += '<envload type="1"  usr="' + this._USR + '" pwd="' + this._PWD + '">';
			xmlstr += '<detection';
			xmlstr += ' chn="' + chk_chn + '"';
			xmlstr += ' sens="' + getSelect("sens") + '"';
			xmlstr += ' mdalarmduration="' + getSelect("mdalarmduration") + '"';
			xmlstr += ' mdalarm="' + getCheckbox("mdalarm") + '"';
			xmlstr += ' mdbuzzer="' + getCheckbox("mdbuzzer") + '"';
			xmlstr += ' vlalarmduration="' + getSelect("vlalarmduration") + '"';
			xmlstr += ' vlalarm="' + getCheckbox("vlalarm") + '"';
			xmlstr += ' vlbuzzer="' + getCheckbox("vlbuzzer") + '"';
			xmlstr += ' />';
			xmlstr += '</envload>';
			xmlstr += '</juan>';
			console.log(xmlstr);
		
		
		 _AJAXget(this.getRequestURL()+'/cgi-bin/gw.cgi?f=j','xml='+xmlstr,false,'',finish);
		
		}
		 $('#Video_detection_settings input:checkbox').prop('checked',false).eq(chn_val-1).prop('checked',true);
		 $('#Video_detection_settings_SelectAll').prop('checked',false);
		async=true;
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
		
		this.Adjust(This._CHN,$('#Alarm_settings'));
		
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
				 warp.find('ul'+warpKey).append('<li onclick="getDataByChn('+j+');"><input type="text" value="'+(j+1)+'" disabled /></li>');
				 warp.find('#Alarm_settings div').eq(j).show();
				
				}
	          		 
			},function(data){
			     
				data2UI(data);
			    warp.find('#Alarm_settings input[type="checkbox"]').prop('checked',false).eq(data.juan.envload.sensor.chn).prop('checked',true);
			},function(){ 
				 warp.find('#Alarm_settings input:checked').prop('disabled',true);
			});
		}
	//设置报警设置信息
	this.dvrAlarmInfoPut = function(){
		
		console.log("-------------------dvrAlarmInfoPut------------------------------");
		var warp = $('#set_content div.dvr_list:visible'),
		    str="saveing",
			This = this,
			object = [],
			getCheckbox = this.getCheckbox,
		    getSelect = this.getSelect,
		    getValue = this.getValue,
			finish = this.checkMultRequests,
		    oHint = showAJAXHint(str).css('top',warp.height() + 46)
			chn_val = parseInt(getValue("chn"));
			 
		     
		dataType='jsonp';  //数据类型
	    jsonp='jsoncallback'; // 回调函数
		
		 warp.find('#Alarm_settings input[type="checkbox"]:visible').each(function(){
          if($(this).prop('checked')==true) {
               object.push($(this).siblings("span").html());
             }
	      console.log(object);
          });
		  
	    _Request=new Array(object.length);
		
		for(var i in object){	
		
		  chk_chn = object[i]-1;
	
		  var xmlstr = '';
		  xmlstr += '<juan ver="0" squ="fastweb" dir="0">';
		  xmlstr += '<envload type="1" usr="' + this._USR + '" pwd="' + this._PWD + '">';
		  xmlstr += '<sensor';
		  xmlstr += ' chn="' + chk_chn + '"';
		  xmlstr += ' mode="' + getSelect("mode") + '"';
		  xmlstr += ' alarmduration="' + getSelect("alarmduration") + '"';
		  xmlstr += ' alarm="' + getCheckbox("alarm") + '"';
		  xmlstr += ' buzzer="' + getCheckbox("buzzer") + '"';
		  xmlstr += ' />';
		  xmlstr += '</envload>';
		  xmlstr += '</juan>';
		  //alert(xmlstr);
	  
		 _AJAXget(this.getRequestURL()+'/cgi-bin/gw.cgi?f=j','xml='+xmlstr,false,'',finish);
		
		}
		
		 $('#Alarm_settings input[type="checkbox"]:visible').prop('checked',false).eq(chn_val-1).prop('checked',true);
		 $('#Alarm_settings_SelectAll').prop('checked',false);
		 
		async=true;
		
	}
		
	//多次请求，提示信息在所有请求都成功之后才能显示成功
     this.checkMultRequests = function(str){
		 
	   console.log(str);
	   console.log(_Request.length);
	   console.log(_Request);
	   
		RequesePush(_Request,str);
		
		var l = _Request.length -1;

		if(!_Request[l]) return; //当ajax请求时，数组的最后一个数据仍为undefined，则退出函数，继续循环执行请求，直到所以请求都完成
			
        var warp = $('#set_content div.dvr_list:visible');
		warp.find('.synCheckboxClick input:checked:visible').prop('disabled',true);
		for(var i=0;i<_Request.length;i++){
			
			if(_Request[i]=='loading_success' || _Request[i]=='save_success' ){
				showAJAXHint('save_success').fadeOut(2000);
				
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
	//保存过程中获取各种input中数据	 
	this.getCheckbox = function (name){
		var warp = $('#set_content div.dvr_list:visible');
		return warp.find('input[data-UI="'+name+'"]').prop("checked")? 1:0;
	}

	this.getSelect=function (name,wrap){
		var warp = $('#set_content div.dvr_list:visible');
		var o = wrap? warp.find('td[data-WARP="'+wrap+'"] input[data-UI="'+name+'"]'):warp.find('input[data-UI="'+name+'"]')
		return  o.attr('data') 
	}

	this.getValue=function (name){
		var warp = $('#set_content div.dvr_list:visible');
		return warp.find('input[data-UI="'+name+'"]').val() || warp.find('input[data-UI="'+name+'"]').attr('data')
	}
	//对copyto的界面调整
	this.Adjust = function(num,str){
		
		if(num >0 && num<=8){
			str.find('tr:gt(0)').hide();
		}else if(num > 8 && num <=16){
			str.find('tr:gt(1)').hide();
		}else if(num > 16 && num<=24){
			str.find('tr:gt(2)').hide();
		}
	}
	//将时间转化为带零的
	this.timeZero =function(str){
		var a =str.split(':');
		 return addZero(a[0])+":"+addZero(a[1])+":"+addZero(a[2])
		}   
		 
	return this;
	}