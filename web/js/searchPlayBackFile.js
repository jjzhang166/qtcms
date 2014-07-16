	var bool = 0, //本地远程回放控制  0为远程 1为本地
		recTotal = 0,  //文件检索总数。
		nowDevID=null; //当前选中设备ID
	//搜索远程录像
	function setDevData2ocx(){

		var b = 1;
		if(bool){  //本地
			if(oPlaybackLocl.SetSynGroupNum(4)){ 
				alert(lang.Failed_to_set_the_number_of_synchronization_group);
				b = 0
			}
		}else{  //远程
			var oDevData=$('#dev_'+nowDevID).data('data'),
			oChannel =oCommonLibrary.GetChannelList(oDevData.dev_id);
			
			/*console.log('-------------参数填充到控件------------------');
			console.log(nowDevID)
			console.log('当前设备');
			console.log(oDevData);
			console.log('当前设备所属通道');
			console.log(oChannel);
			console.log('-------------参数填充到控件------------------')*/

			if(oPlayBack.setDeviceHostInfo(oDevData.address,oDevData.port,oDevData.eseeid)){ 
				alert(lang.Failed_to_set_the_IP_address_or_port_is_not_legal);
				b = 0;
			}

			if(oPlayBack.setDeviceVendor(oDevData.vendor)){
				alert(lang.Failed_to_set_vendor_is_empty);
				b = 0;
			}
			oPlayBack.setUserVerifyInfo(oDevData.username,oDevData.password);
			var intoWindsChl = $("#channelvideo").find('input:checkbox');
			if(intoWindsChl.length != 0){
				var i= 0;
				/*console.log('选中的通道');
				console.log(intoWindsChl);*/
				intoWindsChl.each(function(index){
					//console.log('当前checkbox状态:'+$(this).is(':checked'));
					if($(this).is(':checked') && oChannel[index]){
						//console.log(i+'//'+oChannel[index]);
						if(oPlayBack.AddChannelIntoPlayGroup(i,oChannel[index])){
							b = 0;
						};
						i++;
					}
				});
			}else{
				for(var i=0;i<oDevData.channel_count;i++){
					if(oPlayBack.AddChannelIntoPlayGroup(i,oChannel[i])){
						b = 0;
					}
				}
			}
		}
		return b;
	}
  	function berorSerchShowHint() {
  		$('#fileRec').show().find('span').width(0)
					 .end().find('h5').html('0/0')
					 .end().find('h4').html(lang.Retrieving);
		if(getAudioObj().id=='playbackLocl'){
			$('#fileRec').hide();
		}
  	}
	var typeHint = [];
		typeHint[1] = lang.Timing;
		typeHint[2] = lang.Mobile;
		typeHint[4] = lang.Warning;
		typeHint[8] = lang.Manual;
		typeHint[15] = lang.All;

	var color = [];
		color[1] = '#7BC345';
		color[2] = '#FFE62E';
		color[4] = '#F00';
		color[8] = '#F78445';

	function ocxsearchVideo(){

		recTotal = 0;
		
		berorSerchShowHint();

		getAudioObj().GroupStop();


		var type = $('#type input[data]').attr('data');
		
		var date = $("div.calendar span.nowDate").html();

		var devData = $('div.dev_list li.sel span.device').data('data');

			//console.log($('div.dev_list li.sel span.device'));
			if(!devData){
				$('div.dev_list span.device,li').removeClass('sel');
				devData = $('div.dev_list span.device:first').parent('li').addClass('sel')
							.end().data('data');
			}

		nowDevID = devData.dev_id;

		setDevData2ocx();

		if(bool){  // 本地
			localSearchDevNum=0;

			searchLocalFile(localSearchDevNum,date,type);

			/*var chl ='';
			for (var i=1;i<=devData.channel_count;i++){
				chl+=i+';';
			};
			if(oPlaybackLocl.searchDateByDeviceName(devData.name)){
				alert(T('devData.name',devData.name));
				return;
			}
			oPlaybackLocl.searchVideoFile(devData.name,date,startTime,endTime,chl);*/
		}else{
			// 远程
			var startTime =gettime($('div.timeInput:eq(0) input')) || '00:00:00';
			var endTime =gettime($('div.timeInput:eq(1) input')) || '23:59:59';
			
			var chl = 0;

			for (var i=0;i<devData.channel_count;i++){
				chl += 1 << i;
			};
			var sta = oPlayBack.startSearchRecFile(chl,type,date+' '+startTime,date+' '+endTime)
			console.log(chl+'+'+type+'+'+startTime+'+'+endTime+'搜索文件的状态:'+sta);
			if(sta != 0){
				alert(T('Failed_to_retrieve_video',devData.name,typeHint[type]));
			}
		}
	}

	function searchLocalFile(key,date,type){
		//console.log('当前设备列表------------');
		var oDevList = $('div.dev_list span.device');
		if(key >  (oDevList.length-1))
			return;
		//console.log(localSearchDevNum);
		var type = type ||  $('#type input[data]').attr('data');
		
		var date = date || $("div.calendar span.nowDate").html();

		var name = oDevList.eq(key).data('data').name;

		//console.log('搜索当前设备:'+name+'参数日期为:'+date+'参数文件类型为:'+type+'----------搜索状态为:'+oPlaybackLocl.searchVideoFileEx(name,date,type));
		oPlaybackLocl.searchVideoFileEx(name,date,type);
	}
	function showRecProgress(now){  //回访检索文件进度
			
		now = recTotal;
	//	console.log(now+'//'+recTotal);
		var con = lang.Retrieving,
			p =now/recTotal*100;
		if(recTotal == now){
			con = lang.Retrieval_completed;
		}
		$('#fileRec').find('span').width(p-2)
		             .end().find('h5').html(now+'/'+recTotal)
		             .end().find('h4').html(con);
	}

	function recFileSearchFailCallback(data){
		//console.log(data);
		var hint = [lang.Parameter_error,lang.Connection_Failed,lang.not_complete];
		$('#fileRec h4').html('<h4 style="color:red;">'+hint[data.parm]+'</h4>')
						.end().show();
		setTimeout(function(){
			$('#fileRec').fadeOut(1500);
		},1000);
	}

	function RecfinishCallback(data){ //检索完成回调
		//console.log(data);
		recTotal = data.total ? data.total : 0;	
		
		/*recTotal == 0 && */showRecProgress(recTotal);		
	}

	function initrecFileOcx(obj){
		if(!$('#dev_'+nowDevID)[0]){
			obj.remove();
			$('#fileRec').hide();
			if($('ul.filetree:eq(0) span.device:eq(0)')[0]){
				nowDevID = $('ul.filetree:eq(0) span.device:eq(0)').parent('li').addClass('sel').end().data('data').dev_id;
			}
		}else{
			nowDevID = $('#dev_'+nowDevID).parent('li').addClass('sel').end().data('data').dev_id;
		}

		//console.log('初始化后的设备ID:'+nowDevID);
	}

	/*排序方法	*/
	function TimeAsc(a,b){
		return time2Sec(a.start.split(' ')[1]) - time2Sec(b.start.split(' ')[1]);
	}

	function TimeDes(a,b){
		return time2Sec(b.start.split(' ')[1]) - time2Sec(a.start.split(' ')[1]);
	}

	function ChlAsc(a,b){
		return a.channelnum - b.channelnum;
	}

	function ChlDes(a,b){
		return b.channelnum - a.channelnum;
	}

	function TypeAsc(a,b){
		return a.types - b.types;
	}

	function TypeDes(a,b){
		return b.types - a.types;
	}				
	/*排序方法	*/	

	$(function(){
		oPlayBack.AddEventProc('RecFileInfo','RecFileInfoCallback(data)');  //搜索远程回放回调
		oPlayBack.AddEventProc('recFileSearchFail','recFileSearchFailCallback(data)'); //搜索远程回放失败回调
		oPlayBack.AddEventProc('recFileSearchFinished','RecfinishCallback(data)');  //搜索远程回放完成回调
	})