var oBottom,oPlayBack,oPlaybacKLocl,
	drag_timer = null, //播放时间拖拽的定时器
	oSelected = [], //选中的播放的通道
	recFile=null,	//搜索到的文件,窗口改变的时候重绘搜索文件
	//localRecFile=[],//本地回访搜索文件
	bNoResize=1,   //当前窗口是否在改变
	maxFileEndTime='', //搜索到的文件最大时间
	localSearchDevNum=0; //要搜索的本地回放文件的设备

	$(function(){
		oBottom = $('#operating');
		
		oPlayBack = $('#playback')[0];	

		oPlaybackLocl = $('#playbackLocl')[0];
		
		var channelvideo = $("#channelvideo");
		channelvideo.on('click','input:checkbox',function(event){   //录像文件列表选择通道不能超过4个
			event.stopPropagation();

			//console.log('~~~~~~~~~~~~~~~~~~~~');

			if($(this).prop('checked')){
				oSelected.push(this);
				/*console.log('------------'+oSelected.length);
				console.log(oSelected);	*/	
				if(oSelected.length>4){
					/*console.log('++++++++++++'+oSelected.length);
					console.log(oSelected);*/			
					$(oSelected.shift()).prop('checked',false);
				}
			}else{ 
				oSelected.pop();
			}

			/*console.log('============='+oSelected.length);
			console.log(oSelected);*/

			/* 有时候可以选中超过5个以上. 未找出原因. 以下是修正方案：*/
			if($("#channelvideo :checked").length >4){
				/*console.log('设置失败,修正!');*/
				$("#channelvideo :checkbox").prop('checked',false);
				for( i in oSelected){
					oSelected[i].checked = true;
				}
			}
			/* 有时候可以选中超过5个以上. 未找出原因. 以上是修正方案：*/
		})

		channelvideo.on({  //整个搜索的文件列表事件
			mousedown:function(event){
				try{
					dragStopMove();
					oPlaybackLocl.GroupStop();
					oPlayBack.GroupStop();
					$('#togglePlay').removeAttr('toggle').removeAttr('hasFile').css('background-position','0px 0px');
				}catch(e){
					//alert('try:'+e);
				};
				var left = event.pageX
			    	if(left < 81){
			    		return;
			    	}
			    	if(left > channelvideo.width()){ 
			    		return;
			    	}
				//event.stopPropagation();
				var moveObj = $('div.play_time').css('left',left-1);
				set_drag(80,channelvideo.width()-1,moveObj);
			},
			dblclick:function(){
				playVideo(event);
			}
		})

		channelvideo.on('mouseover','tr',function(){
			var channel_id = $(this).attr('id') ? $(this).attr('id').split('_')[2] : '';
			if(channel_id){
				$('div.dev_list span,li').removeClass('sel');
				$('#channel_'+channel_id).addClass('sel');
			}
			
		})

		$('div.play_time').on({  //文件搜索的下的事件滑动条事件
			dblclick:function(){
				playVideo(event);
			},
			mousedown:function(){
				set_drag(80,channelvideo.width()-1,$('div.play_time'));
			}	
		});

		$('#nowSearchType input:radio').each(function(index){  //全局变量控制远程或本地搜索
			$(this).click(function(){
				
				bool = index;

				$('#channelvideo div.video').remove();

				initOxcDevListStatus();

				//保存当前选中的设备
				nowDevID = $('div.dev_list li.sel span.device').data('data').dev_id;
			})
		})
		
		//return false;

		oPlaybackLocl.AddEventProc('GetRecordFile','RecFileInfoCallback(data)'); //本地回访回调
		/*oPlaybackLocl.AddEventProc('GetRecordFile','RecfinishCallback(data)');*/

		bFullScreen = 0;

		ViewMax();

		initOxcDevListStatus();
	})///
	/*$(window).resize(function(){  //窗口自适应大小
		return;
		dragStopMove();
		var oPlay_time = $('#operating div.play_time'),
			oP = $('#channelvideo'),
			p = oP.width()-81;

		ViewMax();

		// 窗口缩放同步搜索文件UI
		p = (oP.width()-81)/p;

		oPlay_time.css('left',(parseInt(oPlay_time.css('left')) - 79)*p+79);
		
		if(bNoResize){
			noResize();
		}
	})*/

	function ViewMax(){
		var W = $(window).width(),
			H = $(window).height();
			W = W <= 1000 ? 1000: W;
			H = H <= 600 ? 600: H;

		var oView = $('#viewWarp').css({
			width:W-236,
			height:H-oBottom.height()-110
		});

		var oLeft = $('#search_device').css({
			left:oView.width(),
			height:oView.height()+113
		});

		$('div.dev_list').height(oLeft.height()-222);

		oBottom.css({
			width:oView.width(),
			top:oView.height()+80
		});
		
		setTables();
		$('#foot').css({
			top:oView.height()+212
		})
	}

	function togglePlay(){ 
		var obj = $('#togglePlay');
		var to = $('#togglePlay').attr('toggle'),
			hasFile = $('#togglePlay').attr('hasFile');
			speed = $('#togglePlay').attr('speed');
		if(hasFile){
			if(to){
				/*if(speed){
					alert('正常速度');
					GroupSpeedNormal();
				}else{*/
					//alert('继续');
					playAction('GroupContinue');
				//}
			}else{
				//alert('暂停');
				playAction('GroupPause')
			}
		}else{
			//alert('播放');
			playVideo(event);		
		}
	}
	function playVideo(event){
		//alert(event.pageX);
		if(event.pageX<81){
			return;
		}
		
		dragStopMove();
		nowSpeed = 1;
		var obj = $('#togglePlay');
			obj.attr({
				toggle:'1',
				hasFile:'1'
			}).css('background-position','0px'+' '+(-obj.height())+'px');

		oPlaybackLocl.GroupStop();
		oPlayBack.GroupStop();

		var begin = getDragSart($('#channelvideo').width(),$('div.play_time').offset().left+2,$("div.calendar span.nowDate").html()),
			date = $("div.calendar span.nowDate").html(),
			end = date+' '+maxFileEndTime;
			setDevData2ocx();

		//console.log('开始时间:'+begin+'//结束时间'+end);

		var oChannel = $('#dev_'+nowDevID).parent('li').addClass('sel').siblings('li').removeClass('sel')
							.end().end().next('ul').find('span.channel');
		if(bool){ //本地回访
			var k = 0;
			$("#channelvideo").find('input:checked').each(function(){
				//console.log($('#channel_'+$(this).parent('td').parent('tr').attr('id').split('_')[2]));
				var filepath =$('#channel_'+$(this).parent('td').parent('tr').attr('id').split('_')[2]).data('filepath');
				//var filepath = oChannel.eq(index).data('filepath');
				if(filepath){
					//console.log('本地回放文件:'+filepath+'//通道:'+k+'//开始时间:'+begin+'//结束时间:'+end);
					if(oPlaybackLocl.AddFileIntoPlayGroup(filepath,k,begin,end) != 0){
						alert(lang.play_Failed);
					};
				}
				k++;
			});
			oPlaybackLocl.GroupPlay();
		}else{//远程回放
			var type = parseInt($('#type input[data]').attr('data'));
			oPlayBack.GroupPlay(type,begin,end);
		}
		dragStartMove();
	}
	function getDragSart(X2,left,date){
		return  date+' '+returnTime((left-81)/(X2-81)*24*3600);
	}
	function playAction(str){
		var obj = bool ? oPlaybackLocl : oPlayBack; //回放插件对象
			//alert(str+'::当前速度:'+(nowSpeed>1?nowSpeed:1/nowSpeed));
			if(bool && (str == 'GroupSpeedFast' || str == 'GroupSpeedSlow')){
				obj[str](nowSpeed>1?nowSpeed:1/nowSpeed);
			}else{
				obj[str]();
			}
	}

	var nowSpeed = 1;

	function playSpeed(str){
		var show='';
		var max = 2;//bool ? 8 : 2 ;
		if(str){
			nowSpeed = nowSpeed*2;
			nowSpeed = nowSpeed > max ? max : nowSpeed;
		}else{
			nowSpeed = nowSpeed/2;
			nowSpeed = nowSpeed < (1/max) ? (1/max) : nowSpeed;
		}
		if(nowSpeed == 1){
			//alert(nowSpeed+'==');
			playAction('GroupSpeedNormal');
			show='1X';
		}else if(nowSpeed<1){
			playAction('GroupSpeedSlow');
			show='1/'+(1/nowSpeed)+'X';
		}else{
			playAction('GroupSpeedFast');
			show=nowSpeed+'X';
		}
		palybackspeed(show);
	}
	function groupStop(){
		$('#togglePlay').removeAttr('hasFile').removeAttr('toggle').css('background-position','0px 0px');
		dragStopMove();
		var obj = bool ? oPlaybackLocl : oPlayBack;
		obj.GroupStop();
	}

	function palybackspeed(str){
		$('#palybackspeed').html(str);
	}

	/*function VideoData2Ui(obj){  // CGI 数据填充.
		obj.each(function(){ 
			var chlData = $(this).html().split('|'); //disk(int)|session(int)|chn(int)|type(int)|begin(time_t)|end(time_t)
			var startDate = $('div.calendar span.nowDate').html().split('-');
			var start = parseInt(chlData[4])-Date.UTC(startDate[0],parseInt(startDate[1])-1,startDate[2])/1000;
			var p = ($('#channelvideo').width()-80)/(3600*24);
			var width = (chlData[5]-chlData[4])*p
			var left = start*p+81;
			$('<div class="video" style="background:'+color[chlData[3]]+';left:'+left+'px; width:'+width+'px;"></div>').appendTo('#channelvideo tr:eq('+(parseInt(chlData[2]))+')');
		})		
	}*/

	var mergerNum = 0;  // 匹配重合的次数;

	function RecFileInfoCallback(data){

		recFile=data;

		//console.log(recFile);

		initOxcDevListStatus();

		if(bool){
			localSearchDevNum++

			searchLocalFile(localSearchDevNum);
		}
	}

	function Deleteduplicate(data){ // 合并文件
		var maxChl = 0;
		var devFile = [];
		//console.time('遍历最大通道');
		for(i in data){
			var chlData = $.parseJSON(data[i]);
			devFile.push(chlData);
			var nowchl = parseInt(chlData.channelnum);
			maxChl = maxChl > nowchl ? maxChl : nowchl;

		}
		/*console.log('-----最大通道:'+maxChl+'------');
		//console.log(devFile);
		console.timeEnd('遍历最大通道');*/
		var file = [];
		for(i=1;i<=maxChl;i++){  //按通道分类文件
			var chlfile = [];   //对应通道空数组
			console.time('按通道分类文件');
			for(var k=0;k<devFile.length;k++){ //便利所有文件
				var nowfile = devFile[k]
				if(i == nowfile.channelnum){
					chlfile.push(nowfile);
				}
			}
			/*console.timeEnd('按通道分类文件');
			console.log(chlfile);
			console.time('时间生序排列')*/;
			chlfile.sort(TimeAsc);
			/*console.timeEnd('时间生序排列');
			console.time('合并重复文件');*/
			file.push(mergerOrderFile(chlfile));
			/*console.timeEnd('合并重复文件');
			console.log(file);*/
		}
		return file;
	}

	function mergerOrderFile(oChlfile){  //合并时间连续的文件
		if(oChlfile.length <= 1){
			return oChlfile;
		}
		var	n=0,
			m=0,
			chlData=[];
		for(var i=0;i<oChlfile.length;i++){

			if((i+1) < oChlfile.length){
				if(time2Sec(oChlfile[i+1].start) - time2Sec(oChlfile[i].end) <60){  //间隔时间为60秒以内就认为为连续文件.
					m++;
				}else{
					oChlfile[n].end = oChlfile[m+n].end;
					chlData.push(oChlfile[n])	
					n=i+1;
					m=0;
				}
			}
		}
		oChlfile[n].end = oChlfile[m+n].end;
		chlData.push(oChlfile[n]);		
		return chlData;
	}	
	
	function loclFileDataIntoChannel(data){   //那搜索到的原始文件路径填充到对应设备的通道 span.channel上
		/*console.log('文件绑定时收到的数据!');
		console.log(data);
		console.log('===========================');*/
		//console.log(oChannels);
		//for(var k=0;k<data.length;k++){
			var oChannels = $('div.dev_list span.device:eq('+localSearchDevNum+')').next('ul').find('span.channel');
			for(i in data){
				var fileData = $.parseJSON(data[i]);
				if(fileData.filepath){
					var oChannel = oChannels.eq(parseInt(fileData.channelnum -1));
					var filepathArr = oChannel.data('filepath');
						filepathArr = filepathArr ? filepathArr.toString().split(',') : [];
						filepathArr.push(fileData.filepath);
						filepathArr.sort(SortByfileTime).join(',');
					oChannel.data('filepath',filepathArr);
				}
			}
		//}
		return data;
	}

	function RecFileInfo2UI(){
		var oDev = bool ? $('div.dev_list span.device:eq('+localSearchDevNum+')') : $('div.dev_list li.sel span.device');
		//console.log('-------------------当前设备:'+oDev.data('data').name+'----------start---------------');
		var File = Deleteduplicate(recFile);
		//console.time('文件描绘');
		var n=0;
		for( var i=0;i<File.length;i++){
			/*console.log('--------当前填充的通道文件----------');
			console.log(File[i]);*/
			var target = $('#channelvideo tr').not('[id]').eq(0);
			/*console.log('-------------------------初始化的添加对象!----------------------');
			console.log(target);*/
			for(k in File[i]){
				n++;
				var data = File[i][k];
				var start = data.start;
					start=time2Sec(start.split(' ')[1]);
				var end = data.end;
					end = end.split(' ')[1];
					maxFileEndTime = end > maxFileEndTime ? end : maxFileEndTime;
					//console.log(maxFileEndTime);
				var chl = parseInt(data.channelnum -1);

				var ChannelData = oDev.next('ul').find('span.channel').eq(chl).data('data');
				var p = ($('#channelvideo').width()-80)/(3600*24);
				var width = (time2Sec(end)-start)*p;
					width = width < 1 ? 1 : width;
				var left = start*p+81;
				var types = data.types || 8;
					target = target[0] ? target : $('#Rel_channel_'+ChannelData.channel_id);
					/*console.log('-------------------------调整通道ID关联的对象!----------------------');
					console.log(target);*/
						if(!target[0]){
							target = addRecFileTable('id="Rel_channel_'+ChannelData.channel_id+'"',ChannelData.channel_name,($('#channelvideo tr[id]').length+1))
							/*console.log('-------------------------新添加的文件描绘对象通道----------------------');
							console.log(target)*/
						}else{
							target.attr('id','Rel_channel_'+ChannelData.channel_id).find('label').html(ChannelData.channel_name);
						}
						target.attr('title','设备:'+oDev.data('data').name+' 下的通道:'+ChannelData.channel_name);
						//console.log(target);
					//}
				$('<div class="video" style="background:'+color[types]+';left:'+left+'px; width:'+width+'px;"></div>').appendTo(target);
			}
		}

		oSelected = $('#channelvideo tr:lt(4)').find(':checkbox').prop('checked',true).toArray();

		if(!bool){
			showRecProgress(n);
		}
		/*console.timeEnd('文件描绘');
		console.log('-------------------当前设备:'+oDev.data('data').name+'-------------end------------');*/
	}

	function SortByfileTime(a,b){  //文件路径时间升序排列
		var reg = /.*?(\d{6})\.avi/g;
		var a = parseInt(a.replace(reg,'$1'));
		var b = parseInt(b.replace(reg,'$1'));
		return a - b;
	}

	function dragStartMove(){
		var SynTimeUnits = 1000;//nowSpeed<1 ? 1000*nowSpeed:1000/nowSpeed;
		var oPlay = bool ? oPlaybackLocl : oPlayBack;
		//return false;
		var oDrag=$('div.play_time');
		var initleft = parseInt(oDrag.offset().left);
		drag_timer = setInterval(function(){
			var max = $('#channelvideo').width();
			var p = (max-79)/(3600*24);

				max = time2Sec(maxFileEndTime)*p+79 < max ? time2Sec(maxFileEndTime)*p+79 : max;

			var nowPlayd = parseInt(oPlay.GetNowPlayedTime());
			var left = initleft+p*nowPlayd;
			//console.log(bool+'//oxcoPlay:'+$(oPlay).attr('id')+'//初始左边距:'+initleft+'像素//当前已播放时间:'+nowPlayd+'秒//当前走过:'+p*nowPlayd+'像素//当前刷新速度:'+SynTimeUnits+'毫秒//速度'+nowSpeed+'停止播放距离//'+max);
			if(Math.ceil(left) >= Math.floor(max)){
				dragStopMove();
			}
			oDrag.css('left',left);
			//showNowPlayBackTime($('#now_time'),left,max);
		},SynTimeUnits);
	}
	function dragStopMove(){
		//console.log('播放结束');
		clearInterval(drag_timer);
	}
	//回放页面文件显示表格初始化
	function PBrecFileTableInit(){
		$('#channelvideo div.video').remove();
		//var odev = $('div.dev_list li.sel span.channel')
		var oVideoList = $("#channelvideo").html('');
		/*if(odev.length != 0){
			odev.each(function(index){
				var name = $(this).data('data').channel_name;
				var str = index < 4 ? 'checked="checked"' : '';
				addRecFileTable(str,name,index);
			})
		}
		if(odev.length < 4){
			for(var i=2;i<=4;i++)
			addRecFileTable('disabled="disabled"','chl_'+i,i);
		} 

		oSelected = $.makeArray($("#channelvideo input:checkbox:checked"));*/

		for(var i=1;i<5;i++){
			addRecFileTable('',lang['wind']+i,i);	
		}
	}

	function addRecFileTable(id,name,index){
		var a = $('<tr '+id+'><td class="no_border"><input id="chk_'+index+'" checked="checked" type="checkbox" /><label for="chk_'+index+'">'+name+'</label></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td></td><td></tr>').appendTo($("#channelvideo"))

		setTables();
		if($('#channelvideo input:checkbox').length > 4){
			$("#channelvideo input:checkbox:gt(3)").prop('checked',false);
		}
		return a;

	}
	/*function noResize(){
		bNoResize=0;
		$('#channelvideo div.video').remove();
		var winW = $(window).width(),
			winH = $(window).height();
		setTimeout(function(){
			if($(window).width() == winW && $(window).height() == winH){
				if(recFile){
					for(i in recFile){
						RecFileInfo2UI();
					}
				}
			}
			bNoResize=1;
			dragStartMove();
		},200);
	}*/
	function playBackSerchFile(){

		recFile=null;

		PBrecFileTableInit();

		maxFileEndTime = '';

		ocxsearchVideo();
	}

	//初始化控件与文件列表的关系.
	function initOxcDevListStatus(){
		
		if(recFile){
			loclFileDataIntoChannel(recFile);
		}else{
			areaList2Ui();
			
			$('div.dev_list span.device').each(function(){
				$(this).parent('li').on({
					dblclick:function(){ //设备双击开始搜索
						playBackSerchFile();
					},
					click:function(){ //单击同步选中状态
						$('div.dev_list li,span').removeClass('sel');
						$(this).addClass('sel');
					}
				})
			})
		}
		/*areaList2Ui();
		if(bool)
			loclFileDataIntoChannel(recFile);*/

		initrecFileOcx($('#channelvideo div.video'));
		console.time('RecFileInfo2UI');
		RecFileInfo2UI();
		console.timeEnd('RecFileInfo2UI');

		if(bool){
			oPlayBack.style.height='0px';
			oPlayBack.GroupStop();
			oPlayBack.GroupSpeedNormal();
			oPlaybackLocl.style.height='100%';
			$('#type').next('ul.option').find('li:gt(1)').hide();
		}else{
			oPlaybackLocl.style.height='0px';
			oPlaybackLocl.GroupStop();
			oPlaybackLocl.GroupSpeedNormal();
			oPlayBack.style.height='100%';
			$('#type').next('ul.option').find('li').show();
		}	
		
		var objStatus = getAudioObj().GetCurrentState();

		if(objStatus == 2){
			recFile=[];

			palybackspeed('1X');

			dragStopMove();
		}/*else{
			dragStartMove();
			palybackspeed(nowSpeed<1 ? '1/'+1/nowSpeed+'X' : nowSpeed+'X');
		}*/
	}