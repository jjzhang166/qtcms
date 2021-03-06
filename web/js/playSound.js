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
	//console.log('当前对象ID为:'+$(oView).attr('id')+'当前声音切换状态为'+type+'对象切换状态为'+oView.AudioEnabled(type));
	if(oView.AudioEnabled(type)){
		str=lang.Voice_switch_fails;
	}else{
		if(type){
			str=lang.Open_sound;
		}else{
			str=lang.Turn_off_the_sound;
		}
	}
	if(oView.id == 'playback'){
		document.getElementById('playbackLocl').AudioEnabled(type);
	}else if(oView.id == 'playbackLocl'){
		document.getElementById('playback').AudioEnabled(type);
	}

	//oView.enable = type;

	SyncSoundSli(type);
	
	if(oView.id=='previewWindows')
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

function ScreenShot(){  // 截屏
	var data =getAudioObj().ScreenShot(),str;
	if(data.path){
		str =T('window_Screenshot_success',(oPreView.GetCurrentWnd()+1))+data.path+'/'+data.imageName;
	}
	writeActionLog(str);
}

var bFullScreen = 0;

function viewFullScreen(){
	return;
	var obj = $('#viewWarp');
	if(bFullScreen){
		obj.css({
			top:'78px'
		})
		ViewMax();
		bFullScreen = 0;
	}else{
		obj.css({
			top:'0',
			width:'100%',
			height:'100%'
		})
		bFullScreen = 1;
	}
}
$(function(){
	//初始化声音按钮的初始值
	$('#sound').prev('li').prop('soundOn',true);
})