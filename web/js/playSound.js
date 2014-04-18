//sound

function addSoundMove() {  //添加滑动块移动
	$('#sound').on({
		mousedown:function(event){
			var left = event.pageX-$(this).offset().left;
			left = left < 0 ? 0 : left;
			left = left > 100 ? 100 : left;
			var moveObj = $(this).find('div.now_sound').css('left',left-1);		
			$(this).find('p:last').width(left+1);
			set_drag($(this).offset().left,($(this).offset().left+$(this).width()),moveObj);
			getAudioObj().SetVolume(left);
		}
	})
}
function sound(obj){
	var type = obj.prop('soundOn'),
	oView =getAudioObj(),
	str='';
	/*debugData('当前对象ID为:'+$(oView).attr('id')+'当前声音切换状态为'+type+'对象切换状态为'+oView.AudioEnabled(type));*/
	if(oView.AudioEnabled(type)){
		str=lang_trans.Voice_switch_fails;
	}else{
		if(type){
			str=lang_trans.Open_sound;
		}else{
			str=lang_trans.Turn_off_the_sound;
		}
	}
	if(oView.id == 'playback'){
		document.getElementById('playbackLocl').AudioEnabled(type);
	}else if(oView.id == 'playbackLocl'){
		document.getElementById('playback').AudioEnabled(type);
	}

	//oView.enable = type;

	SyncSoundSli(type);
	
	writeActionLog(str);
}
function SyncSoundSli(type){
	var oNext = $('#sound');
	oNext.prev('li').prop('soundOn',!type);
	if(type){
		oNext.children().removeClass('forbidden');
		addSoundMove();
	}else{
		oNext.children().addClass('forbidden');
		oNext.off();
	}
}
function getAudioObj(){   //返回当前页面可控制音量的控件对象。
	var oAudioObj = {};
	if($('#previewWindows')[0]){
		oAudioObj = $('#previewWindows')[0];
	}else{
		if(bool){
			oAudioObj = $('#playbackLocl')[0];
		}else{
			oAudioObj = $('#playback')[0];	
		}
	}
	return oAudioObj;
}