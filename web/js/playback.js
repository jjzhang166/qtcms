var oBottom,oPlayBack,oPlaybacKLocl,
	drag_timer = null, //播放时间拖拽的定时器
	oSelected = [], //选中的播放的通道
	recFile=[],	//搜索到的文件,窗口改变的时候重绘搜索文件
	//localRecFile=[],//本地回访搜索文件
	bNoResize=1,   //当前窗口是否在改变
	maxFileEndTime='00:00:00', //搜索到的文件最大时间
	minFileStartTime='23:59:59', //搜索到的文件最小时间
	localSearchWindNum=0; //要搜索的本地回放文件的设备

	$(function(){

		oBottom = $('#operating');
		
		oPlayBack = $('#playback')[0];

		oPlaybackLocl = $('#playbackLocl')[0];
		
		var channelvideo = $('#channelvideo');

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

		channelvideo.mousedown(function(event){//整个搜索的文件列表事件
			var min = $('table.table .no_border').width(),
				max = channelvideo.find('tr').length > 4 ? channelvideo.width()-17:channelvideo.width();
			try{
				groupStop();
				/*dragStopMove();
				oPlaybackLocl.GroupStop();
				oPlayBack.GroupStop();
				nowSpeed = 1;
				palybackspeed(nowSpeed+'X');*/
				//$('#togglePlay').removeAttr('toggle').removeAttr('hasFile').css('background-position','0px 0px');
			}catch(e){
				//alert('try:'+e);
			};
			var left = event.pageX

	    	if(left < min || left > max){
	    		return;
	    	}
			//event.stopPropagation();
			var moveObj = $('div.play_time').css('left',left-1);

			showNowPlayBackTime($('#now_time'),left-min,max-min);

			set_drag(min,max,moveObj);
		})

		channelvideo.on('mouseover','tr',function(){
			var channel_id = $(this).attr('id') ? $(this).attr('id').split('_')[2] : '';
			if(channel_id){
				$('div.dev_list span,li').removeClass('sel');
				$('#channel_'+channel_id).addClass('sel');
			}
		})

		$('div.play_time').on({ //文件搜索的下的事件滑动条事件
			dblclick:function(){  
				playVideo();
			},
			mousedown:function(){
				var min = $('table.table .no_border').width();
				var max = channelvideo.find('tr').length > 4 ? channelvideo.width()-17:channelvideo.width();
				set_drag(min,max,$(this));
			}
		});

		$('#nowSearchType input:radio').each(function(index){  //全局变量控制远程或本地搜索
			$(this).click(function(){
				
				bool = index;

				searchSTOP=1;

				//保存当前选中的设备
				if(!bool){
					var devData =$('div.dev_list').find('li.sel span.device').data('data') || $('div.dev_list').find('span.channel.sel').parent('li').parent('ul').prev('span.device').data('data');
					nowDevID = devData.dev_id;
				}

				initOxcDevListStatus();

				PBrecFileTableInit();
			})
		})
		
		//return false;

		oPlaybackLocl.AddEventProc('GetRecordFileEx','RecFileInfoCallback(data)'); //本地回访回调
		oPlaybackLocl.AddEventProc('SearchRecordOver','SearchRecordOverCallback(data)');

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
		$('div.play_time').css('left',$('table.table .no_border').width()-1);

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
					dragStartMove();
				//}
			}else{
				//alert('暂停');
				playAction('GroupPause')
				dragStopMove();
			}
		}else{
			//alert('播放');
			playVideo();		
		}
	}
	function playVideo(){
		//console.log(maxFileEndTime+'//'+minFileStartTime);
		if(maxFileEndTime < minFileStartTime)return;

		var date = $("div.calendar span.nowDate").html(),
			begin =$('#now_time').html(),//returnTime(($('div.play_time').offset().left-81)/($('#channelvideo').width()-100)*24*3600), //getDragSart($('#channelvideo').width(),$('div.play_time').offset().left+2,$("div.calendar span.nowDate").html())
			end = date+' '+maxFileEndTime,
			type = parseInt($('#type input[data]').attr('data'));

			begin = begin >= '23:59:59' ? '23:59:59' : begin;

			if(begin>=maxFileEndTime)return;

			groupStop();

			var obj = $('#togglePlay');
			obj.attr({
				toggle:'1',
				hasFile:'1'
			}).css('background-position','0px'+' '+(-obj.height())+'px');

			if(begin<minFileStartTime){
				var min = $('table.table .no_border').width(),
				max = $('#channelvideo tr').length > 4 ? $('#channelvideo').width()-17:$('#channelvideo').width();
				begin = date+' '+minFileStartTime;
				var p = (max-min)/(3600*24);
				$('div.play_time').css('left',p*time2Sec(minFileStartTime)+min);
			}else{
				begin = date+' '+begin;
			}

			setDevData2ocx();

		var oChannel = $('#dev_'+nowDevID).parent('li').addClass('sel').siblings('li').removeClass('sel')
						.end().end().next('ul').find('span.channel');

		console.log(bool+'//开始时间:'+begin+'//结束时间'+end+'//类型'+type);

		if(bool){ //本地回访
			$("#channelvideo").find('input:checked').each(function(){
				/*//console.log($('#channel_'+$(this).parent('td').parent('tr').attr('id').split('_')[2]));
				var filepath =$('#channel_'+$(this).parent('td').parent('tr').attr('id').split('_')[2]).data('filepath');
				//var filepath = oChannel.eq(index).data('filepath');
				if(filepath){
					//console.log('本地回放文件:'+filepath+'//通道:'+k+'//开始时间:'+begin+'//结束时间:'+end);
					if(oPlaybackLocl.AddFileIntoPlayGroup(filepath,k,begin,end) != 0){
						alert(lang.play_Failed);
					};
				}*/
				var wind = $(this).next('label').attr('wind');
				//console.log(wind);
				if(wind){
					var status = oPlaybackLocl.AddFileIntoPlayGroupEx(wind,date,begin.split(' ')[1],maxFileEndTime,type);
					if(status !=0){
						console.log('当前播放窗口为:'+k+'日期为:'+date+'开始时间为:'+begin+'结束时间为:'+end+'文件类型为:'+type+'播放初始化状态:'+status);
						alert(lang.wind+':'+wind+lang.play_Failed);
					}
				}
			});
			oPlaybackLocl.GroupPlay();
		}else{//远程回放
			oPlayBack.GroupPlay(type,begin,end);
		}
		dragStartMove();
	}
	function dragStartMove(){

		if(maxFileEndTime<minFileStartTime) return;

		var channelvideo = $('#channelvideo'),

			min = $('table.table .no_border').width(),

			max = channelvideo.find('tr').length > 4 ? channelvideo.width()-17:channelvideo.width(),

			p = (max-min)/(3600*24),

			SynTimeUnits = 1000,//nowSpeed<1 ? 1000*nowSpeed:1000/nowSpeed;

			oPlay = bool ? oPlaybackLocl : oPlayBack,

			oDrag=$('div.play_time'),

			//FileEndTime = time2Sec(maxFileEndTime)*p+min < max ? time2Sec(maxFileEndTime)*p+min : max,

			initleft = parseInt(oDrag.offset().left);

			oNow = $('#now_time');

			nowTime = oNow.html();

		drag_timer = setInterval(function(){

			var nowPlayd = parseInt(oPlay.GetNowPlayedTime()),
				
				left = initleft+p*nowPlayd;
			
			console.log(bool+'//oxcoPlay:'+$(oPlay).attr('id')+'//初始左边距:'+initleft+'像素//当前已播放时间:'+nowPlayd+'秒//当前走过:'+p*nowPlayd+'像素//当前刷新速度:'+SynTimeUnits+'毫秒//速度'+nowSpeed+'停止播放距离//'+max);

			/*if(Math.ceil(left) >= Math.floor(FileEndTime))
				dragStopMove();*/
			
			oDrag.css('left',left);

			asyncPlayTime2UI(nowTime,nowPlayd,oNow);

		},SynTimeUnits);
	}

	function dragStopMove(){
		//console.log('播放结束');
		clearInterval(drag_timer);
	}

	function asyncPlayTime2UI(now,playdeTime,obj){
		console.log=function(){
			return;
		};
		console.log('当前播放的开始时间:'+now+'已经播放的时间:'+playdeTime);
		var arr = now.split(':');
		arr[0]=parseInt(arr[0]);
		arr[1]=parseInt(arr[1]);
		arr[2]=parseInt(arr[2]);
		console.log(arr);

		arr[2]+=playdeTime;
		console.log('秒1:'+arr[2]);
		var addM = parseInt(arr[2]/60);
		console.log('分1/addM:'+addM);
		arr[2] %=60;
		console.log('秒2:'+arr[2]);

		arr[1] +=addM;
		console.log('分2:'+arr[1]);
		var addH = parseInt(arr[1]/60);
		console.log('时1/addH:'+addH);
		arr[1] %=60;
		console.log('分3:'+arr[1]);

		arr[0]+=addH;
		console.log('时2:'+arr[0]);
		if(arr[0]>=24){
			obj.html('24:00:00');
			//dragStopMove();
			groupStop();
		}else{
			obj.html(addZero(arr[0])+':'+addZero(arr[1])+':'+addZero(arr[2]));	
		}
		console.log(obj.html());
	}
	/*function getDragSart(X2,left,date){
		var time=returnTime((left-81)/(X2-81)*24*3600);

		time = time > minFileStartTime ? time : minFileStartTime;

		return  date+' '+time;
	}*/
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
		var obj = bool ? oPlaybackLocl : oPlayBack;
		obj.GroupStop();
		nowSpeed = 1;
		palybackspeed(nowSpeed+'X');
		dragStopMove();
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

	function RecFileInfoCallback(data){
		for(i in data){
			recFile.push($.parseJSON(data[i]));	
		}
		if(bool && data.index_0){
			RecFileInfo2UI(data)
			console.log('当前窗口:'+(localSearchWindNum)+'的本地路线个文件为----------------');
			console.log(data);
		}

		/*if(!bool)
			console.log(data);*/
		
		localSearchWindNum++;
		if(bool){
			if(localSearchWindNum < 64){
				searchLocalFile(localSearchWindNum);
			}else{
				searchSTOP=1;
			}		
		}else{
			showRecProgress(localSearchWindNum*100);

			console.log(recFile.length+'------------'+recTotal);
			searchSTOP && RecFileInfo2UI(recFile);
		}

		searchSTOP && file2UIFinish();
	}

	function file2UIFinish(){
		$('#channelvideo tr').not('[id]').find('input').prop('disabled',true);
		var n=0;
		$('#channelvideo tr[id]').each(function(){
			n<4 && $(this).find('input').prop('checked',true);
			n++
		})
		oSelected = $('#channelvideo input:checked').toArray();
	}

	function Deleteduplicate(data){ // 去重复
		var maxChl = 0,
			devFile = [],
			file = [];
		if(bool){
			var chlfile = [];
			for(i in data){
				var chlData = typeof(data[i]) == 'object' ? data[i] : $.parseJSON(data[i]);
				chlfile.push(chlData);
			}
			chlfile.sort(TimeAsc);
			/*console.log('---------同一通道下的文件-------------');
			console.log(chlfile);

			console.log('---------合并完成接收到的文件-------------');
			console.log(mergerOrderFile(chlfile));*/
			file.push(mergerOrderFile(chlfile));
			//file.push(chlfile);
			/*console.log('---------合并后的文件-------------');
			console.log(file);*/
		}else{
			//console.time('遍历最大通道');
			for(i in data){
				var chlData = typeof(data[i]) == 'object' ? data[i] : $.parseJSON(data[i]);
				devFile.push(chlData);
				var nowchl = parseInt(chlData.channelnum);
				maxChl = maxChl > nowchl ? maxChl : nowchl;
			}
			/*console.log('-----最大通道:'+maxChl+'------');
			//console.log(devFile);
			console.timeEnd('遍历最大通道');*/
			for(i=1;i<=maxChl;i++){  //按通道分类文件
				var chlfile = [];    //对应通道空数组
				//console.time('按通道分类文件');
				for(var k=0;k<devFile.length;k++){ //便利所有文件
					var nowfile = devFile[k]
					if(i == nowfile.channelnum){
						chlfile.push(nowfile);
					}
				}
				/*console.timeEnd('按通道分类文件');
				console.log(chlfile);
				console.time('时间生序排列');*/
				chlfile.sort(TimeAsc);
				/*console.timeEnd('时间生序排列');
				console.time('合并重复文件');*/
				file.push(mergerOrderFile(chlfile));
				/*console.timeEnd('合并重复文件');
				console.log(file);*/
			}
		}
		//console.log(file);
		/*console.log('---------合并后返回的文件-------------');
		console.log(file);*/
		return file;
	}

	function mergerOrderFile(oChlfile){  //合并时间连续的文件
		/*console.log('合并开始接受到的文件————————————');
		console.log(oChlfile);*/
		if(oChlfile.length < 2 || !oChlfile){
			return oChlfile;
		}

		var	n=0,
			m=0,
			chlData=[];

		for(var i=0;i<oChlfile.length;i++){
			if((i+1) < oChlfile.length){
				if((time2Sec(oChlfile[i+1].start.split(' ')[1]) - time2Sec(oChlfile[i].end.split(' ')[1]) < 60) && (oChlfile[i].types == oChlfile[i+1].types)){  //间隔时间为60秒以内就认为为连续文件.
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
		/*console.log('合并完成后返回的的文件————————————');
		console.log(oChlfile);*/
		return chlData;
	}	
	
	function loclFileDataIntoChannel(data){   //那搜索到的原始文件路径填充到对应设备的通道 span.channel上
		return;
		/*console.log('文件绑定时收到的数据!');
		console.log(data);
		console.log('===========================');*/
		//console.log(oChannels);
		//for(var k=0;k<data.length;k++){
			var oChannels = $('div.dev_list span.device:eq('+localSearchWindNum+')').next('ul').find('span.channel');
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

	function RecFileInfo2UI(filedata){
		console.time('--接收到的文件回调描绘时间段---'+filedata.length);
		var oList = $('div.dev_list'),

			channelvideo = $('#channelvideo'),

			oDev = bool ?  oList.find('span.device:eq('+localSearchWindNum+')')[0] : (oList.find('li.sel span.device')[0] || oList.find('span.channel.sel').parent('li').parent('ul').prev('span.device')[0]),

			oFileUIwarp = channelvideo.find('tr'),

			oDev = $(oDev),
		/*console.log(oDev);
		console.log('---------描绘接受到的文件------------------------');
		console.log(filedata)*/	
		//console.time('---------合并接受到的文件------------------------');
			File = Deleteduplicate(filedata),
		/*console.timeEnd('---------合并接受到的文件------------------------');
		console.time('--接收到合并的文件回调描绘时间段---'+File.length);*/

			min = $('table.table .no_border').width(),

			max = channelvideo.find('tr').length > 4 ? channelvideo.width()-17:channelvideo.width(),

			p = (max-min)/(3600*24),

			nowTime = renewtime().split('  ')[1];

		/*console.log('接收到的文件进行合并后的文件----------------------------');
		console.log(File);*/

		for( var i=0;i<File.length;i++){
			/*console.log('--------当前填充的通道文件----------');
			console.log(File[i]);*/
			var str = '';
			/*console.log('-------------------------初始化的添加对象!----------------------');
			console.log(target);*/
			for(k in File[i]){
				var data = File[i][k];
				//console.log(data);
				var start = data.start.split(' ')[1];

					minFileStartTime = start < minFileStartTime ? start : minFileStartTime;

					start = time2Sec(start);

				var end = data.end.split(' ')[1];

					maxFileEndTime = end > maxFileEndTime ? end : maxFileEndTime;

			  		//本地回放录像文件的结束时间不能大于当前描绘的时间.
					/*if( bool && end > nowTime){
						end = nowTime
					}*/

					//console.log(maxFileEndTime);
				var width = (time2Sec(end)-start)*p-1;
					width = width < 1 ? 1 : width;
				var left = start*p+min+1;
				var types = data.types || 8;
				if(bool){ // 本地回放
					var target = oFileUIwarp.eq(localSearchWindNum);
					var wind = lang['wind']+' '+(parseInt(data.wndId)+1);
					target.attr({
						id:'wind_'+data.wndId,
						title:_T('wind')+': '+(parseInt(data.wndId)+1)+'下的 '+$("div.calendar span.nowDate").html()+'日的所有本地录像文件'
					}).find('label').html(wind).attr('wind',data.wndId);
				}else{
					var chl = parseInt(data.channelnum -1),
						ChannelData = oDev.next('ul').find('span.channel').eq(chl).data('data');
					var target = oFileUIwarp.eq(ChannelData.channel_number).attr({
							id:'Rel_channel_'+ChannelData.channel_id,
							title:'设备:'+oDev.data('data').name+' 下的通道:'+ChannelData.channel_name
						}).find('label').html(ChannelData.channel_name).end();
				}

				str+='<div class="video" style="background:'+color[types]+';left:'+left+'px; width:'+width+'px;"></div>';
			}
			$(str).appendTo(target);
		}
		//console.log('minFileStartTime:'+minFileStartTime+'-----------------maxFileEndTime:'+maxFileEndTime);
		/*console.timeEnd('--接收到合并的文件回调描绘时间段---'+File.length);*/
		console.timeEnd('--接收到的文件回调描绘时间段---'+filedata.length);
	}

	function SortByfileTime(a,b){  //文件路径时间升序排列
		var reg = /.*?(\d{6})\.avi/g;
		var a = parseInt(a.replace(reg,'$1'));
		var b = parseInt(b.replace(reg,'$1'));
		return a - b;
	}

	//回放页面文件显示表格初始化
	function PBrecFileTableInit(){
		$('table.table').width('100%');
		var currOdevData = $('div.dev_list').find('li.sel span.device').data('data') || $('div.dev_list').find('span.channel.sel').parent('li').parent('ul').prev('span.device').data('data');
		var initWind = bool ? 64 : currOdevData.channel_count;
		console.log('+++++++++++++要初始化的文件列表数量+++++++++++++++++++:'+initWind);
		initWind+=1;
		initWind = initWind < 5 ? 5 : initWind;

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
		var str = '';
		var str2 = bool ? lang['wind']:lang['Channel'];
		for(var i=1;i<initWind;i++){
			/*var chk='checked="checked"';
			if(i>4)
				chk='';*/
			str+= '<tr><td class="no_border"><input id="chk_'+i+'" type="checkbox" /><label for="chk_'+i+'">'+str2+' '+i+'</label></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td></td><td></tr>';
		}

		$(str).appendTo(oVideoList);

		if(initWind>5 ){
			$('table.table').eq(0).width($('table.table').eq(0).width()-17);
		}

		setTables();
	}

	function addRecFileTable(id,name,index){
		var chk='checked="checked"';
		/*if(index>4)
			chk='';*/
		var a = $('<tr '+id+'><td class="no_border"><input id="chk_'+index+'" type="checkbox" /><label for="chk_'+index+'">'+name+'</label></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td></td><td></tr>').appendTo($("#channelvideo"))

		setTables();
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
		console.log('当前搜索状态:'+searchSTOP);
		if(searchSTOP){
			searchSTOP = 0;
		}else{
			console.log('当前搜索状态:'+searchSTOP+'------------------------正在搜索');
			return;
		}


		groupStop();
		
		localSearchWindNum=0;

		recFile=[];

		maxFileEndTime='00:00:00';
		minFileStartTime='23:59:59';

		$('#channelvideo').find('div.video').remove()
						  .end().find('tr').removeAttr('id title')
						  		.find('input').removeProp('disabled').removeProp('checked');

		!bool && PBrecFileTableInit();						  		

		ocxsearchVideo();

	}

	function SearchRecordOverCallback(data){
		//console.log(data);
		if(data.searchResult){
			searchSTOP = 1;
		}
	}

	//初始化控件与文件列表的关系.
	function initOxcDevListStatus(){
			searchSTOP=1;
		
		/*if(recFile){
			loclFileDataIntoChannel(recFile);
		}else{*/
			//console.time('--重画设备列表菜单--');
			areaList2Ui('','','closed');
			//console.timeEnd('--重画设备列表菜单--');
			
			//console.time('--设备列表事件添加--');
			$('div.dev_list span.device').each(function(){
				$(this).parent('li').on({
					dblclick:function(){ //设备双击开始搜索
						/*if(bool)return;
						PBrecFileTableInit();*/
						playBackSerchFile();
						/*//保存当前选中的设备
						nowDevID = $(this).find('span.device').data('data').dev_id;*/
					},
					click:function(){ //单击同步选中状态
						if(bool)return;
						$('div.dev_list li,span').removeClass('sel');
						nowDevID = $(this).addClass('sel').find('span.device').data('data').dev_id;
						//PBrecFileTableInit();
					}
				})
			})
			//console.timeEnd('--设备列表事件添加--');
		//}
		/*areaList2Ui();
		if(bool)
			loclFileDataIntoChannel(recFile);*/

		initrecFileOcx($('#channelvideo div.video'));

		groupStop();
		//console.time('--本地远程控件调整--');
		if(bool){
			oPlayBack.style.height='0px';
			//oPlayBack.GroupStop();
			oPlayBack.GroupSpeedNormal();
			oPlaybackLocl.style.height='100%';
			$('#type').next('ul.option').find('li:gt(1)').hide();
		}else{
			oPlaybackLocl.style.height='0px';
			//oPlaybackLocl.GroupStop();
			oPlaybackLocl.GroupSpeedNormal();
			oPlayBack.style.height='100%';
			$('#type').next('ul.option').find('li').show();
		}	
		//console.timeEnd('--本地远程控件调整--')
		//console.time('--本地远程控件状态同步--')
		var objStatus = getAudioObj().GetCurrentState();

		if(objStatus == 2){
			recFile=null;

			palybackspeed('1X');

			dragStopMove();
		}/*else{
			dragStartMove();
			palybackspeed(nowSpeed<1 ? '1/'+1/nowSpeed+'X' : nowSpeed+'X');
		}*/
		//console.timeEnd('--本地远程控件状态同步--');
	}
	function setTables(){   // 回放页面底部表格最大化相应调整
		var W =  $('table.table').width()-70,
			p=W/24;
		$('table.table td').not('.no_border').css('width',p)
		$('table.table .no_border').width($('table.table').width()-p*24);
	}