// JavaScript Document
function connectEvent(id,e,proc){
	document.getElementById(id).addEventListener(e,proc,true);
}

function disconnectEvent(id,e,proc){
	document.getElementById(id).removeEventListener(e,proc,true);
}

function AddActivityEvent(e,proc){
	try
	{
		qob.AttachEvent(e,proc);
	}catch(err)
	{
		if (err.name == 'ReferenceError')
		{
			setTimeout(function(){AddActivityEvent(e,proc);},100);
		}
	}
}

function ttst(){alert(1);}

function subViewMsg(msg,param){
	try{
		window[msg.Proc](param)
	}catch(e){
		alert(e);
	};
}
var str = document.getElementById('commonLibrary').getLanguage();

var oScript = document.createElement('script');
	oScript.type='text/javascript';
	oScript.src = 'lang/trans_'+str+'.js';

var node = document.getElementById('add_lang');
node.parentNode.insertBefore(oScript,node.nextSibling);

function translate(key){//语言切换
	var str = key;
		str = lang_trans ? lang_trans[key] : key;
	document.write(str);
}

