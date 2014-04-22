	var bool = 0, //本地远程回放控制  0为远程 1为本地
		recTotal = 0;  //文件检索总数。
//搜索远程录像
function setDevData2ocx(){
		var oChannel =$('div.dev_list li.sel span.channel'),
			oDevData=$('div.dev_list li.sel span.device').data('data');

		var b = true;
		if(bool){
			if(oPlaybackLocl.SetSynGroupNum(4)){ 
				alert(lang.Failed_to_set_the_number_of_synchronization_group);
				b = false
			}
		}else{
			if(oPlayBack.setDeviceHostInfo(oDevData.address,oDevData.port,oDevData.eseeid)){ 
				alert(lang.Failed_to_set_the_IP_address_or_port_is_not_legal);
				b = false;
			}

			if(oPlayBack.setDeviceVendor(oDevData.vendor)){
				alert(lang.Failed_to_set_vendor_is_empty);
				b = false;
			}
			oPlayBack.setUserVerifyInfo(oDevData.username,oDevData.password);
			var intoWindsChl = $("#channelvideo").find('input:checkbox');
			if(intoWindsChl.length != 0){
				var i= 0;
				intoWindsChl.each(function(index){
					if($(this).is(':checked')){
						if(oPlayBack.AddChannelIntoPlayGroup(i,oChannel.eq(index).data('data').channel_id)){
							b = false;
						};
						i++;
					}
				});
			}else{
				for(var i=0;i<oDevData.channel_count;i++){
					if(oPlayBack.AddChannelIntoPlayGroup(i,oChannel.eq(i).data('data').channel_id)){
						b = false;
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
		typeHint[1] = lang.Time;
		typeHint[2] = lang.Move;
		typeHint[4] = lang.Warning;
		typeHint[8] = lang.Manual;
		typeHint[15] = lang.All;
	function ocxsearchVideo(){

		recTotal = 0;
		
		berorSerchShowHint();
		
		if(!$('div.dev_list li.sel')[0]){
			$('div.dev_list li:eq(1)').addClass('sel');
		}

		var devData = $('div.dev_list li.sel span.device').data('data');
		var type = $('#type span').attr('value') || 0;
			type = type == 0 ? 15 : 1 << type;
		var date = $("div.calendar span.nowDate").html();
		var startTime =gettime($('div.timeInput:eq(0) input')) || '00:00:00';
		var endTime =gettime($('div.timeInput:eq(1) input')) || '23:59:59';
		setDevData2ocx();
		/*show(chl+'+'+type+'+'+startTime+'+'+endTime);
		alert(oPlayBack.startSearchRecFile(chl,type,startTime,endTime));*/
		if(bool){
			var chl ='';
			for (var i=1;i<=devData.channel_count;i++){
				chl+=i+';';
			};
			if(oPlaybackLocl.searchDateByDeviceName(devData.name)){
				alert(T('devData.name',devData.name));
				return false;
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