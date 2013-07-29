// JavaScript Document
function connectEvent(id,e,proc){
	document.getElementById(id).addEventListener(e,proc,true);
}

function disconnectEvent(id,e,proc){
	document.getElementById(id).removeEventListener(e,proc,true);
}

function ttst(){alert(1);}