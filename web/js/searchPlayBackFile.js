	var bool = 0, //本地远程回放控制  0为远程 1为本地
		recTotal = 0,  //文件检索总数。
		nowDevID=null; //当前选中设备ID
//搜索远程录像
function setDevData2ocx(){
		var oDevData=$('#dev_'+nowDevID).data('data'),
			oChannel =oCommonLibrary.GetChannelList(oDevData.dev_id);

		var b = 1;
		if(bool){
			if(oPlaybackLocl.SetSynGroupNum(4)){ 
				alert(lang.Failed_to_set_the_number_of_synchronization_group);
				b = 0
			}
		}else{
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
				intoWindsChl.each(function(index){
					if($(this).is(':checked')){
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

		var devData = $('div.dev_list li.sel span.device').data('data');

		nowDevID = devData.dev_id;

		var type = $('#type span').attr('value');
		
		var date = $("div.calendar span.nowDate").html();
		var startTime =gettime($('div.timeInput:eq(0) input')) || '00:00:00';
		var endTime =gettime($('div.timeInput:eq(1) input')) || '23:59:59';
		setDevData2ocx();
		/*console.log(chl+'+'+type+'+'+startTime+'+'+endTime);
		alert(oPlayBack.startSearchRecFile(chl,type,startTime,endTime));*/
		if(bool){
			var chl ='';
			for (var i=1;i<=devData.channel_count;i++){
				chl+=i+';';
			};
			if(oPlaybackLocl.searchDateByDeviceName(devData.name)){
				alert(T('devData.name',devData.name));
				return;
			}
			oPlaybackLocl.searchVideoFile(devData.name,date,startTime,endTime,chl);
		}else{
			var chl = 0;

			for (var i=0;i<devData.channel_count;i++){
				chl += 1 << i;
			};
			if(oPlayBack.startSearchRecFile(chl,type,date+' '+startTime,date+' '+endTime)!=0){
				alert(T('Failed_to_retrieve_video',devData.name,typeHint[type]));
			}
		}
	}
	function showRecProgress(now){  //回访检索文件进度
			
			now = recTotal

		var con = lang.Retrieving,
			p =now/recTotal*100;
		if(recTotal == now){
			con = lang.Retrieval_completed;
		}
		$('#fileRec').find('span').width(p-2)
		             .end().find('h5').html(now+'/'+recTotal)
		             .end().find('h4').html(con);
	}

	function RecfinishCallback(data){ //检索完成回调
		recTotal = data.total ? data.total : 0;	
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