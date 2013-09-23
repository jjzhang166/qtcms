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