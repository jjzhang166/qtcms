// JavaScript Document
var nThreadId,operationType,AJAXHint;
var ONVIF = function(ip,port,usr,pwd){
	
	this._IP = ip;  //ip地址
	this._PORT = port; //端口
	this._USR = usr; //用户名
	this._PWD = pwd;  //密码
	
	this.OnvifDeviceInfo2UI = function(){
		 console.log("-------------------OnvifDeviceInfo2UI------------------------------");
		emptyOnvifinfo();
		onvifSetting.setOnvifDeviceParam(this._IP,this._PORT,this._USR,this._PWD);
		onvifSetting.getOnvifDeviceBaseInfo();
	 }
	
	this.onvifDeviceEncoderInfo2UI = function(){
	   emptyOnvifinfo();
	    console.log("-------------------onvifDeviceEncoderInfo2UI------------------------------");

	    onvifSetting.setOnvifDeviceParam(this._IP,this._PORT,this._USR,this._PWD);
	    
		onvifSetting.getOnvifDeviceEncoderInfo();
		// console.log(data);

	}
	
	this.onvifDeviceEncoderInfoPut = function(){
	    console.log("-------------------onvifDeviceEncoderInfoPut------------------------------");
        
		var index = $('#onvifStream').attr('data'),
		    fps = $('#onviffps').val(),
			bps = $('#onvifbps').val(),
			codeformat = $('#onvifcodeFormat').attr('data');
			interval = $('#onvifinterval').val(),
		    profile = $('#onvifprofile').attr('data'),
			resolution = $('#onvifresolution').val();
		var resArry = resolution.split('x');			
	    onvifSetting.setOnvifDeviceParam(this._IP,this._PORT,this._USR,this._PWD);
		onvifSetting.setOnvifDeviceEncoderInfo(parseInt(index,10),parseInt(resArry[0],10),parseInt(resArry[1],10),fps,bps,codeformat,interval,profile);
		console.log(index+' '+resArry[0]+' '+resArry[1]+' '+fps+' '+bps+' '+codeformat+' '+interval+' '+profile);

	}
	
	this.onvifnetworkInfo2UI = function(){
		 emptyOnvifinfo(); 
		 console.log("-------------------onvifnetworkInfo2UI------------------------------");
		
		 onvifSetting.setOnvifDeviceParam(this._IP,this._PORT,this._USR,this._PWD);
	   
		 onvifSetting.getOnvifDeviceNetworkInfo();
		
	}
	
	this.onvifnetworkInfoPut = function(){
		console.log("-------------------onvifnetworkInfoPut------------------------------");
		var wrap = $('#set_content div.onvif_list:visible');
		
		var sip = $('#onvifip').val(),
		    smac = $('#onvifsMac').val(),
			sgateway = $('#onvifsGateway').val(),
			smask = $('#onvifsMask').val(),
			sdns = $('#onvifsDns').val();
       console.log(sip+' '+smac+' '+sgateway+' '+smask+' '+sdns);
		onvifSetting.setOnvifDeviceParam(this._IP,this._PORT,this._USR,this._PWD);
	    onvifSetting.setOnvifDeviceNetWorkInfo(sip,smac,sgateway,smask,sdns);
		  
	}
	
	return this;
}
function datatoui(num){
    
	 var defaultwrap = $('#set_content div.switch:visible');
		defaultwrap.find('ul[data-UI="resolutionArray"] li').remove();
	
	num = parseInt(num,10);
	 var wrap = $('#set_content div.switch:visible');
	var objData =  wrap.find('td[data-UI="Streamitem"] li').eq(num).data('data')
	 
	for(var i in objData){
	  
		if(typeof objData[i] == 'object'){
			if(objData[i] instanceof Array){
				
				  var targ = wrap.find('ul[data-UI~="'+i+'"]');
				  var len=objData[i].length;
				  
				  if(i=='profileArray'){
				   
					for(var j = 0;j<len;j++){
					     var str3;
						  switch(objData[i][j]){
						   case '0':
							str3="NVP_H264_PROFILE_BASELINE";
							break;  
						   case '1':
							str3="NVP_H264_PROFILE_MAIN";
							break; 
						   case '2':
							str3="NVP_H264_PROFILE_EXTENDED";
							break; 	
						   case '3':
							str3="NVP_H264_PROFILE_HIGH";
							break; 	
						    case '4':
							str3="NVP_H264_PROFILE_NR";
							break; 	
						}
					  $('<li style="width:220px;"><input type="text" value="'+str3+'" style="width:220px;" data='+objData[i][j]+' disabled/></li>').appendTo(targ);	
				  }
						
				}else{
				  for(var j = 0;j<len;j++){
					  
					$('<li style="width:220px;"><input type="text" value="'+objData[i][j]+'" style="width:220px;" disabled/></li>').appendTo(targ);	
				  }
				}
		   }else{
			   
				  $('#'+i).val(objData[i].value).attr({
					  'min':objData[i].min,
					  'max':objData[i].max
				  });
		  }
		}else{
			if(i=='onvifcodeFormat'){
			   var str2;
				switch(objData[i]){
				   case '0':
				    str2="NVP_VENC_JPEG";
					break;  
				   case '1':
				    str2="NVP_VENC_MPEG4";
					break; 
				   case '2':
				    str2="NVP_VENC_H264";
					break; 	
			    }
				$('#'+i).val(str2).attr('data',objData[i]);
			}else if(i=='onvifprofile'){
				  var str4;
				switch(objData[i]){
				   case '0':
					str4="NVP_H264_PROFILE_BASELINE";
					break;  
				   case '1':
					str4="NVP_H264_PROFILE_MAIN";
					break; 
				   case '2':
					str4="NVP_H264_PROFILE_EXTENDED";
					break; 	
				   case '3':
					str4="NVP_H264_PROFILE_HIGH";
					break; 	
					case '4':
					str4="NVP_H264_PROFILE_NR";
					break; 	
			    }
				$('#'+i).val(str4).attr('data',objData[i]);
			}else{
			  $('#'+i).val(objData[i]);
			}
		}
    }
	
}

function emptyOnvifinfo(){
	
   var defaultwrap = $('#set_content div.switch:visible');
		defaultwrap.find('li').not('.bb').remove();
		defaultwrap.find('input').not('.bb').val('').attr({
			'min':'',
			'max':'',
			'data':''
	    });	
	 $('#set_content .right_content:eq(1) div.switch').find('input').css('border',0).remove('b');
}
//onvif 设置回调函数
function operationStart(data){
  console.log('----operationStart----');
  console.log(data);
  AJAXHint='';
  var wrap = $('#set_content div.onvif_list:visible');
  if(data.operationType=='0'||data.operationType=='2'||data.operationType=='4'){
	 showAJAXHint("loading").css('top',wrap.height()+46); 
  }else{
	 showAJAXHint("saveing").css('top',wrap.height()+46);  
  }	
  nThreadId = data.nThreadId;
  operationType=data.operationType;
}
function operationReturnInfo(data){
  console.log('----operationReturnInfo----');
  console.log(data);
  if(data.nThreadId != nThreadId || data.operationType != operationType)return;
  
  var wrap = $('#set_content div.onvif_list:visible');
   
  if(data.operationType=='0'){
	  if(data.status=='1' || !data.info){
		 AJAXHint="loading_fail";
		  return;
	   }
	  networkinfo(data);
  }else if(data.operationType=='1'){
	   if(data.status=='1'){
		 AJAXHint="save_fail";
		  return;
	   }
  }else if(data.operationType=='2'){
	  if(data.status=='1' || !data.info ){
		 AJAXHint="loading_fail";
		  return;
	   }
		encodeinfo(data); 
  }else if(data.operationType=='3'){
	   if(data.status=='1'){
		 AJAXHint="save_fail";
		  return;
	   }
  }else if(data.operationType=='4'){
	  if(data.status=='1' || !data.info ){
		  AJAXHint="loading_fail";
		  return;
	   }
	 onvifDevinfo(data);
  }
}
function operationEnd(data){
  console.log('----operationEnd----');
  console.log(data);	
  
  if(data.nThreadId != nThreadId || data.operationType != operationType)return;
    var wrap = $('#set_content div.onvif_list:visible');
   if(AJAXHint){
	  showAJAXHint(AJAXHint).css('top',wrap.height()+46);
	  return;   
	}
  if(data.operationType=='0'||data.operationType=='2'||data.operationType=='4'){
	 showAJAXHint("loading_success").css('top',wrap.height()+46).fadeOut(2000); 
  }else{
	 showAJAXHint("save_success").css('top',wrap.height()+46).fadeOut(2000);  
  }	
}
function onvifDevinfo(data){
 
  var xmlDoc = bexml(data.info);
 
  var model = xmlDoc.getElementsByTagName('OnvifDeviceInfo')[0].getAttribute("model"),
       devname = xmlDoc.getElementsByTagName('OnvifDeviceInfo')[0].getAttribute("devname"),
	   sw_version = xmlDoc.getElementsByTagName('OnvifDeviceInfo')[0].getAttribute("sw_version"),
	   firmware = xmlDoc.getElementsByTagName('OnvifDeviceInfo')[0].getAttribute("firmware"),
	   hw_version = xmlDoc.getElementsByTagName('OnvifDeviceInfo')[0].getAttribute("hw_version"),
	   hwid = xmlDoc.getElementsByTagName('OnvifDeviceInfo')[0].getAttribute("hwid"),
	   sn = xmlDoc.getElementsByTagName('OnvifDeviceInfo')[0].getAttribute("sn"),
	   manufacturer = xmlDoc.getElementsByTagName('OnvifDeviceInfo')[0].getAttribute("manufacturer"),
	   sw_builddate = xmlDoc.getElementsByTagName('OnvifDeviceInfo')[0].getAttribute("sw_builddate");
	  
	 var str1 = "{'model':model,'devname':devname,'sw_version':sw_version,'firmware':firmware,'hw_version':hw_version,'hwid':hwid,'sn':sn,'manufacturer':manufacturer,'sw_builddate':sw_builddate}";
	 var data1 = eval('('+str1+')');	 
	  for(var i in data1){

		 $('#onvif'+i).val(data1[i]);

	   }

}

function encodeinfo(data){
	
		var wrap = $('#set_content div.onvif_list:visible');
		var target = wrap.find('td[data-UI="Streamitem"] ul');
		
		var xmlDoc = bexml(data.info);
	
		var encodeNum = xmlDoc.getElementsByTagName("OnvifStreamEncoderInfo")[0].getAttribute("itemNum");
		
		for(var i=0;i<parseInt(encodeNum,10);i++){
			var str;
			var j = xmlDoc.getElementsByTagName("StreamItem")[i].getAttribute("index");
		  if(i==0){
		     str = _T("Main_stream");
		  }else{
			str = _T('Sub_stream')+j;  
		  }
		  var fps = xmlDoc.getElementsByTagName("StreamItem")[i].getAttribute("enc_fps"),
		      bps = xmlDoc.getElementsByTagName("StreamItem")[i].getAttribute("enc_bps"),
		      codeFormat = xmlDoc.getElementsByTagName("StreamItem")[i].getAttribute("codeFormat"),
			  resolution = xmlDoc.getElementsByTagName("StreamItem")[i].getAttribute("resolution"),
		      fpsmin =  xmlDoc.getElementsByTagName("enc_fps")[i].getAttribute("min"),
		      fpsmax =  xmlDoc.getElementsByTagName("enc_fps")[i].getAttribute("max"),
			  bpsmin =  xmlDoc.getElementsByTagName("enc_bps")[i].getAttribute("min"),
			  bpsmax =  xmlDoc.getElementsByTagName("enc_bps")[i].getAttribute("max"),
			  interval = xmlDoc.getElementsByTagName("StreamItem")[i].getAttribute("enc_interval"),
			  intervalmin =  xmlDoc.getElementsByTagName("enc_interval")[i].getAttribute("min"),
			  intervalmax =  xmlDoc.getElementsByTagName("enc_interval")[i].getAttribute("max"),
			  resolutionNum =  xmlDoc.getElementsByTagName("resolution")[i].getAttribute("itemNum"),
			  profile = xmlDoc.getElementsByTagName("StreamItem")[i].getAttribute("enc_profile"),
			  profileNum = xmlDoc.getElementsByTagName("enc_profile")[i].getAttribute("itemNum");
			  
			  resolutionNum = parseInt(resolutionNum,10);
			  profileNum = parseInt(profileNum,10);
		  var resolutionArray =[resolutionNum],profileArray =[profileNum];
		  for(var k=0;k<resolutionNum;k++){
			  resolutionArray[k] = xmlDoc.getElementsByTagName("resolution")[i].getElementsByTagName("item")[k].firstChild.nodeValue;
		  }
		  for(var g=0;g<profileNum;g++){
			 profileArray[g] =  xmlDoc.getElementsByTagName("enc_profile")[i].getElementsByTagName("item")[g].getAttribute("profile");
		  }
         // console.log('fps:'+fps+' bps:'+bps+' codeFormat:'+codeFormat+' resolution:'+resolution+' fpsmin:'+fpsmin+' fpsmax:'+fpsmax+' bpsmin:'+bpsmin+' bpsmax:'+bpsmax+' intervalmin:'+intervalmin+' intervalmax:'+intervalmax); 
		    var str1 = "{'onviffps':{'value':fps,'min':fpsmin,'max':fpsmax},'onvifbps':{'value':bps,'min':bpsmin,'max':bpsmax},'onvifcodeFormat':codeFormat,'onvifprofile':profile,'onvifinterval':{'value':interval,'min':intervalmin,'max':intervalmax},'onvifresolution':resolution,'resolutionArray':resolutionArray,'profileArray':profileArray}";
			var data1 = eval('('+str1+')');
			console.log(data1);
		   $('<li style="width:220px;" onclick="datatoui('+i+')"><input type="text" value="'+str+'" style="width:220px;" data='+i+' disabled/></li>').appendTo(target).data('data',data1);
	   } 
		 datatoui(0); 
		 $('#onvifStream').val(_T("Main_stream")).attr('data',0);
}

function networkinfo(data){
   
   var xmlDoc = bexml(data.info);
   var mask = xmlDoc.getElementsByTagName('NetworkInfo')[0].getAttribute("sMask"),
       gateway = xmlDoc.getElementsByTagName('NetworkInfo')[0].getAttribute("sGateway"),
	   dns = xmlDoc.getElementsByTagName('NetworkInfo')[0].getAttribute("sDns"),
	   mac = xmlDoc.getElementsByTagName('NetworkInfo')[0].getAttribute("sMac");
	  
	 var str1 = "{'sMask':mask,'sGateway':gateway,'sDns':dns,'sMac':mac,'ip':data.ip}";
	 var data1 = eval('('+str1+')');	 
	   for(var i in data1){

		 $('#onvif'+i).val(data1[i]);

	   }

}

function bexml(str){
  var xmlDoc;
  	 if(window.DOMParser){
		  parser=new DOMParser();
		  xmlDoc=parser.parseFromString(str,"text/xml");
		 
	  }else{// Internet Explorer
		xmlDoc=new ActiveXObject("Microsoft.XMLDOM");
		xmlDoc.async="false";
		xmlDoc.loadXML(str);
	  }
  	return xmlDoc;
}




