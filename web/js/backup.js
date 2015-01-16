var oPlayBack={},  // 远程回访控件对象
    oPlaybacKLocl={},
	oBackup={},
	oBackupLocal={},    // 备份控件对象
	now = 0,	   // 当前下载的第几个文件
	recFile=[],    // 搜索文件
	usedUid = [],
	nowPage=-1,
	LocalFlag = true,//本地文件搜索是否有文件的标记，1表示没有 ，0表示有
	maxFileEndTime='00:00:00', //搜索到的文件最大时间
	minFileStartTime='23:59:59', //搜索到的文件最小时间
	perPageNum=100;
    
	$(function(){
		oPlayBack = $('#playback')[0];
		oPlaybackLocl = $('#playbackLocl')[0];
		oBackup = $('#backup')[0];
		autoSearchDev = $('#atuoSearchDevice')[0];
         
		 var username = autoSearchDev.getCurrentUser();
		  username && $('.top_nav div span:eq(1)').html(username);
		 
		$('tbody.synCheckboxClick').on('click','input:checkbox',function(){
			if(oBackup.getProgress() != 0){
				return;
			}
		})
       
		$('tbody.synCheckboxClick').each(function(){
			$(this).SynchekboxClick();
		})
		
		//文件排序
		$('#search_result thead:eq(0) td:gt(1)').mousedown(function(){ 
			$(this).css('background-position','0 -20px');
		}).mouseup(function(){ 
			$(this).css('background-position','0 0');
			var type = $(this).attr('sort');
			if(type && recFile){ // 文件排序重画UI
				$('#search_resultFile').html('');
				var sorType='',
					node = $(this);

				if(type == 'Time'){
					node = $(this).parent('tr').find('td[sort="Time"]');
				}
				if($(this).attr('sorType')){
					sorType = 'Des';
					node.removeAttr('sorType');
				}else{
					sorType = 'Asc';
					node.attr('sorType',1);
				}
				reSort_2UI(type+sorType);			
			}
		}).mouseout(function(){
			$(this).css('background-position','0 0');
		})
         $('#nowSearchType input:radio').each(function(index){  //全局变量控制远程或本地搜索
			$(this).click(function(){

				
				$('#fileRec').stop(true,true).hide();
				 
			
				bool = index ? 0:1;

				searchSTOP=1;

				//保存当前选中的设备
				if(!bool){
					var devData =$('div.dev_list').find('li.sel span.device').data('data') || $('div.dev_list').find('span.channel.sel').parent('li').parent('ul').prev('span.device').data('data');
					nowDevID = devData.dev_id;
					var obj =$('#search_resultFile tr');
					if(obj.length != 0){
						obj.filter(function(){
							return !$(this).find(':checkbox').is(':checked');
						}).remove();//远程备份中正在下载的文件不被删除.
					}

				}else{
				   var localobj =$('#windowFile tr');
					   localobj.find('div.canvas').remove();
					   localobj.find(':checkbox').prop('checked',false);
					   localobj.find(':checkbox').prop('disabled',false);  
				}

				initOxcDevListStatus();
				PBrecFileInit();

				
			})
		})
		
		$('#timebar1,#timebar2').on({ //时间棒滑动事件
			mousedown:function(){
				var windowfile = $('#windowFile');
				var min = $('table.table_backup .no_border').width();
				var max = windowfile.width()-17;
				 timebar_drag(min,max,$(this));	
			}
		});
		
		
		contentMax();
		initOxcDevListStatus();
		PBrecFileInit();
	 
		oBackup.AddEventProc('RecordDirPath','getDirCallback(data)');  //远程备份获取备份路径
		oBackup.AddEventProc('BackupStatusChange','BackupStatusChangeCallback(data)'); //远程备份下载状态
		oBackup.AddEventProc('progress','progressCallback(data)'); //远程备份下载进度
	    
		autoSearchDev.AddEventProc('useStateChange','useStateChange(ev)');
		autoSearchDev.startGetUserLoginStateChangeTime();
		AddActivityEvent('Validation','Validationcallback(data)');
		
		$('.hover').each(function(){  // 按钮元素添加鼠标事件对应样式
		   var action = $(this).attr('class').split(' ')[0];
		    addMouseStyle($(this),action,1<<4);
	    })
	})///

	$(window).resize(contentMax);
	
	function timebar_drag(X1,X2,oDrag){  // 备份页面的拖拽条
		  var oNow=$('#now_time');	
		  var left;
		  
		  $(document).mousemove(function(event){
			  left = event.pageX;
			  left = left < X1 ? X1 : left;
			  left = left > X2 ? X2 : left;
		  
			  showNowPlayBackTime(oNow,left-X1,X2-X1);
			  
			  oDrag.attr('title',returnTime(((left-X1)/(X2-X1))*24*3600));
				  
			  oDrag.css('left',left-1.5);
			  
		  }).mouseup(function(){
			  $(this).off();
		  })
    }
	
	function Validationcallback(data){ //id按钮权限验证
	  //console.log(data);
		if(data.ErrorCode=="1"){
			autoSearchDev.showUserLoginUi(336,300);
		}else if(data.ErrorCode=="2"){
			showLimitTips();
			var timer =setTimeout(function(){
				closeMenu();
				clearTimeout(timer);
			},1000);
		}
	}
   
	function RecFileInfoCallback(data){
		//console.time('---------------搜索到文件回调---------------');
		var file =[];
		for (var i in data){
			   recFile.push($.parseJSON(data[i]));
			   file.push($.parseJSON(data[i]));
		    }	
			
		var wnum=localSearchWindNum;
  
		 if(bool && data.index_0){
			//console.log('当前窗口:'+(parseInt((localSearchWindNum))+1));
			 LocalFlag = false;
			 RecLocalFile2UI(file);
			 showLocalRecProgress(wnum++);
			//console.log('当前窗口:'+(localSearchWindNum)+'的本地路线个文件为----------------');
			//console.log(data);
		}
		//console.log(recTotal+'//'+recFile.length);

		//if(recTotal==recFile.length)
        localSearchWindNum++;
		
		if(bool){
        
			if(localSearchWindNum < 49){
				
				searchLocalFile(localSearchWindNum);
				
			}else{
				searchSTOP=1;
			}
			
			 localSearchWindNum >= 49&&searchSTOP && file2UIFinish();		
		}else{
			
			
			var l = $('#page span').length;
			$('<span onclick="selectPage('+l+',this)">'+(l+1)+'</span>').appendTo($('#page'));
	
			if(nowPage == -1){
				//console.log('-----------第一页--------')
				$('#page span:first').addClass('sel');
				nowPage=0;
				selectPage(nowPage);
			}
			
			showRecProgress((l+1)*100);
			
		}
		
        
		//console.timeEnd('---------------搜索到文件回调---------------');

	}
     
	 function RecLocalFile2UI(file){
		 
		 //console.log(file);
		var windowFile = $('#windowFile'),
		    oFileUIwarp = windowFile.find('tr');

	    var min = $('table.table_backup .no_border').width(),

			    max = windowFile.find('tr').length > 4 ? windowFile.width()-17:windowFile.width(),

			    p = (max-min)/(3600*24),
            
			tdH = $('table.table_backup .no_border').height(),
			 
			nowTime = renewtime().split('  ')[1];
			
        var color = [];
			color[1] = '#F00';
			color[2] = '#0000FF';
			color[4] = '#33CC00';
			color[8] = '#FFFF00';
		var target = oFileUIwarp.eq(localSearchWindNum);
		var winNum = localSearchWindNum;
		var wind = lang['wind_']+(parseInt(winNum)+1);       
			target.attr({
				id:'wind_'+localSearchWindNum,
				title:_T('Date')+':'+$("div.calendar span.nowDate").html()+' '+_T('wind')+': '+(parseInt(winNum)+1)+' '+_T('All_Local_Video_File')
			}).find('label').html(wind).attr('wind',parseInt(winNum));
		
		var id = "mycanvas"+winNum;
		var canvas = document.getElementById(id);
		var context = canvas.getContext("2d");
		//console.log("窗口"+id);

		
		for( var i=0;i<file.length;i++){
			
				var data = file[i];
				//console.log(data);
				var start = data.start.split(' ')[1];

					minFileStartTime = start < minFileStartTime ? start : minFileStartTime;

					start = time2Sec(start);

				var end = data.end.split(' ')[1];

					maxFileEndTime = end > maxFileEndTime ? end : maxFileEndTime;

				var width = (time2Sec(end)-start)*p;
			
				var left = start*p+min;
				var types = data.types || data.type;

				//console.log(localSearchWindNum+' '+data.wndId+' '+data);
					
               context.fillStyle =color[types];
			 //console.log("color"+color[types]);
		      //context.rect(left-min,0,width,tdH);
			    context.fillRect(left-min,0,width,tdH);
		}
	 }
	 
	 function showLocalRecProgress(now){

		if(now != 0 && now>=49){
			
			now=49;
		}
		var con = lang.Retrieving,
			p =now/49*100,
			str = (now/49*100).toString().slice(0,5);
			str = str == 'NaN'?'':str+'%';

		if(49 == now ){
			con = lang.Retrieval_completed;
		}

		$('#fileRec').stop(true,true).find('span').show().width(p-2)
		             .end().find('h5').html(str)
		             .end().find('h4').html(con);
		return str; 
		
	}
	
	function reSort_2UI(sortfn){
		var obj =$('#search_resultFile tr');
		if(obj.length != 0){
			obj.filter(function(){
				return !$(this).find(':checkbox').is(':checked');
			}).remove();//远程备份中正在下载的文件不被删除.

			obj.has(':checked').each(function(){
				usedUid.push($(this).attr('uid'));
			})
		}
		var fn = window[sortfn],
			start = nowPage*perPageNum;
			nowPageFile = recFile.slice(start,(start+perPageNum+1)).sort(fn);
		//console.log('----------文件重绘第:'+nowPage+'页-'+start+'到'+(start+perPageNum+1)+'------');
		//console.log(nowPageFile);
		RecFile2UI(nowPageFile);
	}

	function RecFile2UI(nowPageFile){
		/*console.log(nowPageFile);
		console.log(splitBY1Hour(nowPageFile));*/
		nowPageFile = splitBY1Hour(nowPageFile);
		//console.time('+++++++开始绘制搜索到的文件UI+++++++++'+nowPageFile.length);

			
		var sDevName = $('div.dev_list li.sel span.device').data('data').device_name,
			str ='',
			oParent = $('#search_resultFile');
        var bo = document.getElementById('commonLibrary').getLanguage() =='en_PR',
		    isJalaali = document.getElementById('commonLibrary').getIsPersian(),
			isChecked = bo&&isJalaali;
			
		for(var i=0;i<nowPageFile.length;i++){ 
			var b = 1;
			var data = nowPageFile[i];
			var uid = sDevName+'_'+data.channelnum+'_'+data.types+'_'+data.start+'_'+data.end;
			var type =typeHint[data.types];
			var typeColor = color[data.types];
            var start = isChecked ? solar_to_jalaali(data.start.split(' ')[0])+ ' ' + data.start.split(' ')[1] : data.start ;
		    var end = isChecked ? solar_to_jalaali(data.end.split(' ')[0])+ ' ' + data.end.split(' ')[1] : data.end ;
			for(var k=0;k<usedUid.length;k++){
				if(usedUid[k] == uid){
					b=0;
				}
			}
			
			if(b){
				str+='<tr uid="'+uid+'"><td><input type="checkbox" /></td><td>'+sDevName+'</td><td>'+data.channelnum+'</td><td><span class="type"><span class="hint">'+type+'</span><span class="color" style="background:'+typeColor+'"></span></span></td><td>'+start+'</td><td>'+end+'</td><td class="progess"><div><span></span></div><a></a></td></tr>';
			}
		}
		var obj = $(str).appendTo(oParent);

		var warp = $('#file_warp');

		if($('#search_resultFile').height() > warp.height() && !warp.attr('b')){
			theadtbody(warp.find('thead td'),warp.prev('table').find('td'));
			warp.attr('b',0);
		}
		/*console.time('++++++文件列表完成后数据绑定+++++++++');	
		for(var i=0;i<recFile.length;i++){ 
			var data = recFile[i];
			var uid = sDevName+'_'+data.channelnum+'_'+data.start+'_'+data.end;
			//console.log(uid);
			//obj.is('[uid="'+uid+'"]').find(':checkbox').data('data',data);
			obj.each(function(){
				if($(this).attr('uid') == uid){
					$(this).find(':checkbox').data('data',data);
				}
			})
		}
		console.timeEnd('++++++文件列表完成后数据绑定+++++++++');	*/

		//console.timeEnd('+++++++开始绘制搜索到的文件UI+++++++++'+nowPageFile.length);
	}

	function splitBY1Hour(arr){
		var a=[];
		for(var i=0,j=arr.length;i<j;i++){
			
			//console.log(arr[i]);
			if(time2Sec(arr[i].end.split(' ')[1]) - time2Sec(arr[i].start.split(' ')[1]) > 3600){
				/*console.log('当前文件时间大于一小时');
				console.log(arr[i]);*/
				a = a.concat(SPLIT(arr[i]));
			}else{
				a.push(arr[i]);
			}
		}

		return a;

		function SPLIT(obj){
			var date = obj.start.split(' ')[0] || obj.end.split(' ')[0],
				start = obj.start.split(' ')[1],
				end = obj.end.split(' ')[1],
				aa=[],
				st = start;

			for(var j=0;j<24;j++){

				var o = {},

					newed = returnTime(parseInt((time2Sec(st)/3600),10)*3600+3599);

					newed = newed > end ? end : newed;

				//console.log(newed);

				if(newed >= end){
					//console.log(st+'//'+newed+'-----------------'+(newed >= end));
					//console.log(aa);
					return aa;
				}

				o.channelnum = obj.channelnum;
				o.filename = obj.filename;
				o.index = obj.index;
				o.start = date +' '+st;
				o.types = obj.types;
				o.end = date +' '+ newed;
				st = returnTime(parseInt(time2Sec(newed))+1);

				aa.push(o);
			}
		}

	}

	function contentMax(){
		var H = $(window).height();
		var W = $(window).width();
			W = W <=1000 ? 1000 : W;
			H = H <=600 ? 600 : H;

		var main = $('.search_result').css({
			height: H - 138,
			width: W - 238
		});

		$('#search_device').css({
			left:main.width()+2,
			height:H-116
			
		})
		$('#foot').css({
			top:main.height() + 110
		})
		$('#top').width(main.width()+238);
		$('div.dev_list').height(main.height()-200);
		$('#file_bottom').css({
			width:main.width(),
			top:main.height()+78
		});
	}

	function getDir(){
		if($('#search_resultFile input:checked').length == 0){
			return;
		}
		oBackup.ChooseDir();
	};

	var path = '',
		now = 0,
		timer = null;
	
	function getDirCallback(data){ 
		now = 0;
		path = data.path;
		if(path == '')
			return;

		var oList = $('#search_result tr').find('td.progess span').stop(true,true).css('width',0).end().has(':checked');
		if(!oList.length || oList.length == 0){
			return;
		}

		startDownload();
	}

	function startDownload(){
		/*if(now >= oList.length){
			show('所有选择的文件下载完成!');
			now=0;
			return false;
		}*/
		var oList = $('#search_result tr')/*.find('td.progess span').stop(true,true).css('width',0).end()*/.has(':checked');
		var oDevData = $('div.dev_list li.sel span.device').data('data');
		var fileData = oList.eq(now).attr('uid').split('_');
		/*console.log(fileData);
		console.log('--------选中设备--------')*/
		/*console.log($('div.dev_list li.sel span.device'));
		console.log(oDevData);
		console.log('--------选中通道--------')
		console.log(oList.eq(now));
		console.log(fileData);*/
		console.log('开始下载当前第'+(now+1)+'/'+oList.length+'个文件!-----------设备地址:'+oDevData.address+'--端口--'+oDevData.port+'--易视ID--'+oDevData.eseeid+'--通道号--'+(fileData[1]-1)+'--文件类型--'+fileData[2]+'--设备名--'+oDevData.device_name+'--文件开始时间--'+fileData[3]+'--文件结束时间--'+fileData[4]+'--保存路径--'+path);
		if(oBackup.startBackup(oDevData.address,oDevData.port,oDevData.eseeid,(fileData[1]-1),fileData[2],oDevData.device_name,fileData[3],fileData[4],path) != 0){
			alert(T('File_download_failed',(parseInt(now,10)+1)));
			now++;
			startDownload();
		}
		/*oList = oList.filter(function(index){
			if(index == 0){ 
				var fileData = $(this).data('data');
				
			}
			return index != 0;
		})*/
	}
	function progressCallback(data){
		//console.log('当前第'+(now+1)+'个文件的下载进度为--------'+data.parm+'---------自动抛出进度---------');
		var oWarp = $('#search_result tr').has('input:checked').eq(now);
			oWarp.find('a').one('click',function(){
				backupstop();
				//show('当前第'+now+'个任务已被终止!');
				return;
			})
		var width = (data.parm/100)*oWarp.find('div').width();
			oWarp.find('.progess span').stop(true,true).animate({width:width},1000);
	}
	
	/*function syndownload(){   //同步下载文件状态.
		return;
		var i=0;
		var oWarp = $('#search_result tr').has('input:checked').eq(now);
			oWarp.find('a').one('click',function(){
				backupstop();
				//show('当前第'+now+'个任务已被终止!');
				return;
			})
		timer = setInterval(function(){
			i++;
			var p = oBackup.getProgress();
			var width = p*oWarp.find('div').width();
			oWarp.find('.progess span').stop(true,true).animate({width:width},1000);
			//console.log('当前第'+(now+1)+'个文件的下载进度为--------'+p+'---------手动获取进度---------::所耗时间为'+i+'秒!');
			if(p == 1){
				//clearInterval(timer);
				$('#search_result tr').has('input:checked').eq(now).prop('checked',false).find('span').width(oWarp.find('div').width());
			}
		},1000);
		//show('正在下载当前选中文件的第'+now+'个!');
	}*/

	function BackupStatusChangeCallback(data){
		var oList = $('#search_result input:checked'),
			oWarp = oList.eq(now).parent('td').parent('tr'),
			types = data.types;
		console.log('当前第'+(now+1)+'/'+oList.length+'下载文件状态:'+data.types+'----------------------');
		/*if(types == 'startBackup'){
			syndownload();
		}else{*/

			if(types == 'backupFinished'){ 
				console.log('=================当前下载的第'+(now+1)+'个文件下载成功==========================');
				oWarp.find('.progess span').css('width','100%');
				     //.end().find('input').prop('checked',false);
			}

			if(types=='insufficient-disk'){
				backupstop();
				oList.prop('checked',false);
				alert(lang.Insufficient_Disk_Space);
				return;
			}

			if( types == 'fail'){
				console.log('=================当前下载的第'+(now+1)+'个文件下载失败==========================');
				oWarp.find('.progess').find('span').css('background-color','#ccc')
									  .end().find('div').css('border-style','dashed');	
			}
			if( types == 'stopBackup'){
				console.log('=================当前下载的第'+(now+1)+'个文件下载停止==========================');
				//clearInterval(timer);
				now++;
				if(now == oList.length){
					backupstop();
					return;
				}
				oWarp.find('a').off();
				startDownload();
			}
		/*}*/
	}

	function backupstop(){
		console.log('所有选择的文件下载完成!!!');
		oBackup.stopBackup();
		now=0;
		$('#search_result input:checked').prop('checked',false);
		//clearInterval(timer);
	}

	function backupSearchFile(){
		//console.log('~~~~~~~~~~~~~~~~~~~~当前搜索状态:'+searchSTOP+'~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~');
		//bool = 0;
		if(searchSTOP){
			searchSTOP = 0;
		}else{
			//console.log('当前搜索状态:'+searchSTOP+'------------------------正在搜索');
			return;
		}
		$('.search_result :checkbox').prop('checked',false);
		$('#file_warp').removeAttr('b');
		usedUid = [];
		recFile=[];
		nowPage=-1;
		$('#page').html('');
		localSearchWindNum=0;
		console.log('++++++++开始搜索+++++++++++');
		if(bool){
			var localobj =$('#windowFile tr');
			  localobj.find('div.canvas').remove();//远程备份中正在下载的文件不被删除.
			  addcanvas();
		}else{
			var obj =$('#search_resultFile tr');
			if(obj.length != 0){
				obj.filter(function(){
					return !$(this).find(':checkbox').is(':checked');
				}).remove();//远程备份中正在下载的文件不被删除.
	
				/*obj.has(':checked').each(function(){
					usedUid.push($(this).attr('uid'));
				})*/
			}
		  }
		
		
		ocxsearchVideo();
	}

	function initOxcDevListStatus(){
		areaList2Ui(1);

		$('div.dev_list span.device').each(function(){
			$(this).parent('li').on({
				dblclick:function(){
					if(bool)return;
					checkUserRightdiv(1<<4,0,"backupSearchFile");
			     },
				click:function(){
					if(bool)return;
					$('div.dev_list li').removeClass('sel');
					$(this).addClass('sel');
				}
			})
		})

        $('#type').next('ul.option').find('li').show();
		if(bool){
			$('#type').next('ul.option').find('li').show();
			$('#type').next('ul.option').find('li:gt(1):lt(3)').hide();
		}else{
			$('#type').next('ul.option').find('li').show();
			$('#type').next('ul.option').find('li:gt(4)').hide();
		}	
       
		
        
		initrecFileOcx($('#search_resultFile tr').filter(function(){return $(this).find(':checkbox').not('checked')}));
	}
     function PBrecFileInit(){
		 var index = bool?0:1;
		   $('.search_result').hide();
	      $('.search_result').eq(index).show();	 
		  if(index==0){
			  
			   var local = $('#windowFile').css({
     		     height:$('.search_result').height()-24,
		     })
		      $('#windowFile').prev('table').find('tr').width(local.width()-17);
			  
		    theadtbody($('#table_backup thead td'),$('#windowFile thead:first td'));
			  
			setTables();
			
			addcanvas();

			$('#timebar1').css('left',$('#windowFile .no_border').width());
	
			$('#timebar2').css('left',$('#windowFile tr').width());
		  }else{
			
		
		    $('#file_warp').height($('.search_result').height() -18).find('thead td:gt(3)').not(':last').width(($('#file_warp').width()-700)/2);
			
            theadtbody($('#file_warp thead td'),$('#search_result thead:first td'));
		 
			  
		  }
	 }
	 function setTables(){   // 回放页表格最大化相应调整
		var W =  $('table.table_backup').width()-70,
			p=W/24;
		$('table.table_backup td').not('.no_border').css('width',p)
		$('table.table_backup .no_border').width($('table.table_backup').width()-p*24);
	}
	
	function addcanvas(){
	  for(var j=0;j<49;j++){
		var target = $('#windowFile tr').eq(j),
			minL =target.find('td.no_border').width();
	   $('<div class="canvas" style="position:absolute;top:0px;left:'+(minL)+'px;width:'+(target.width()-minL)+'px;height:'+target.height()+';"><canvas id="mycanvas'+j+'" width="'+(target.width()-minL)+'" height="'+target.height()+'"></canvas></div>').appendTo(target);
	  }
		
	}
	function addtables(){
		var fragment = document.createDocumentFragment();
		var str = lang['wind_'];
		for(var i=0; i<49; i++){
			
			var newitem = $('<tr><td class="no_border"><input id="win_'+i+'" type="checkbox" /><label for="win_'+i+'">'+str+i+'</label></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td></tr>');
			fragment.appendChild(newitem);
	    }
		
		 $("#windowFile")[0].appendChild(fragment);
		
    }
	function file2UIFinish(){
	 
	  $('#windowFile tr[id]').find('input').prop('disabled',false);
	  $('#windowFile tr').not('[id]').find('input').prop('disabled',true);
	}
	function selectAll(){
		var oCheckbox = bool ? $('#windowFile :checkbox').not(':disabled') : $('#search_resultFile :checkbox');
		var b = true;
		oCheckbox.each(function(){
			if(!$(this).is(':checked')){
				b = true;
			}else{
				b = false;
			}

		})
		oCheckbox.prop('checked',b);
	}

	function selectPage(num,oSel){
		$(oSel).addClass('sel').siblings('span').removeClass('sel');
		var obj =$('#search_resultFile tr');
		if(obj.length != 0){
			obj.filter(function(){
				return !$(this).find(':checkbox').is(':checked');
			}).remove();//远程备份中正在下载的文件不被删除.

			obj.has(':checked').each(function(){
				usedUid.push($(this).attr('uid'));
			})
		}
		usedUid = unique(usedUid);
		/*console.log('----------------重新选择的页数:'+num+'--------------------------');
		console.log(usedUid);*/
		nowPage=num;
		reSort_2UI('ChlAsc');
	}
	function unique(arr){ 
		var str = []; 
		for(var i = 0,len = arr.length;i < len;i++){ 
			!RegExp(arr[i],'g').test(str.join(",")) && (str.push(arr[i])); 
		} 
		return str; 
	} 
	//本地搜索回调
	function SearchRecordOverCallback(data){
	   	if(LocalFlag){
			$('#fileRec').stop(true,true).find('span').show().width(0)
		             .end().find('h5').html('')
		             .end().find('h4').html(lang.Retrieval_completed);
		    
		}else{
		  if(data.searchResult=='SUCCESS')
		     showLocalRecProgress(49);
		  else if(data.searchResult=='INCOMPLETE'){
			$('#fileRec h4').html('<h4 style="color:red;">'+lang.not_complete+'</h4>')
		      .end().find('h5').html('')
			  .end().show();
		    setTimeout(function(){
			  $('#fileRec').stop(true,true).fadeOut(1500);
		    },1000);
		  }
		  
		}

		LocalFlag=true;
    }
	function ThrowExceptionCallback(data){
	    console.log(data);	
	 	
	}
	
	//验证用户是否有权限
  function checkUserRight(uicode,uisubcode){
	  //console.log('uicode:'+uicode+' uisubcode:'+uisubcode);
	  var itema= autoSearchDev.checkUserLimit(uicode.toString(2),uisubcode);
	// console.log("当前用户"+autoSearchDev.getCurrentUser()+" 登录状态："+itema);
		return itema;
 }
  function checkUserRightBtn(uicode,uisubcode,fn,num){
	 // console.log('uicode:'+uicode+' uisubcode:'+uisubcode);
	  var itema= autoSearchDev.checkUserLimit(uicode.toString(2),uisubcode);
	  // console.log("当前用户"+autoSearchDev.getCurrentUser()+" 登录状态："+itema);
		if(itema==0){
			window[fn](num);
		}else if(itema==1){
			//autoSearchDev.showUserLoginUi(336,300);
		}else{
		   showLimitTips();
		}
 }
 function checkUserRightdiv(uicode,uisubcode,fn,num){
	 
	 var itema = checkUserRight(uicode,uisubcode);
			if(itema==0){
				window[fn](num);
			}else if(itema==1){
			  var show = autoSearchDev.showUserLoginUi(336,300);
			  if(show==0){
				   checkUserRightdiv(uicode,uisubcode,fn,num);
			  } 
		    }else{
		        showLimitTips();
		    }
	 
 }
 //用户登录状态回调函数
    function useStateChange(ev){
	//	console.log(ev);
		if(ev.status==0){
		 $('.top_nav p span:eq(1)').html(ev.userName);	
		}else{
		  $('.top_nav p span:eq(1)').html(_T("not_Login"));
		}
	}
	
	 function lock(){
	autoSearchDev.showUserLoginUi(336,300); 
 }