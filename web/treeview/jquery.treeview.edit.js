//系统设置添加设备
function add() {
	var branches = $("<li><span class='file'>"+$('#txt_usr').val()+"</span></li>").appendTo(".ul1");
	$("#browser").treeview({
		add: branches
	});
};
//treeview方法
function tree(frameId){
$('#'+frameId+'').treeview();
$('#'+frameId+'').bind("contextmenu", function(event) {
	if ($(event.target).is("li") || $(event.target).parents("li").length) {
		$('#'+frameId+'').treeview({
			remove: $(event.target).parents("li").filter(":first")
		});
		return false;
	}			  
});			 
}						
//用户设备以及操作节点
/*function addNode(txt_eseeid,txt_nick,txt_usr,txt_pwd,dev_id,modify,del){
	var branches =$('<li id = "'+dev_id+'" class="closed"><span class="folder" ><span class = "modify" onclick="modify_device_link(this);" href="javascript:;" modify_id="'+dev_id+'"></span><span  class="del" href="javascript:;" onclick = "del_device_link('+dev_id+');"></span><a class="dev_name" onclick="javascript:;" style="line-height:20px;">'+txt_nick+'</a></span><ul class = "'+txt_eseeid+'"></ul></li>').appendTo('.ul1');
		$('ul.filetree').treeview({add:branches});
		var obj = [];
		obj.push(txt_eseeid);
		obj.push(txt_usr);
		obj.push(txt_pwd);
		obj.push(dev_id);										
		branches.data('data',obj);
}
function addChl(cam_id,chlname,esee_id){
	var branches = $('<li id = "'+cam_id+'"><span class="file"><span class="modify" onclick="modify_camName_link(this);"></span><a class="chl_name">'+chlname+'</a></span></li>').appendTo('.'+esee_id+'');
	$('ul.filetree').treeview({add:branches});
	var obj = [];
	obj.push(cam_id);
	obj.push(chlname);	
	obj.push(esee_id);	
	branches.find('span.modify').data('data',obj);
	}
//视屏预览 用户设备节点
function previewNode(txt_nick,esee_id,area_id){
	if(area_id == ''){
		var target = $('.ul1');
	}else{
		var target = $('#area ul[pid ='+area_id+']');
		};
		$('<li class="closed" ><span class="folder" ><a class = "open_all" esee_id = "'+esee_id+'" onclick="open_close_all_chl(this,0);" state = "0">'+txt_nick+'</a></span><ul  id="folder21" class = '+esee_id+' ></ul>').appendTo(target);
	}	
//视屏预览 用户设备所属摄像头节点	
function chlNode(esee_id,chlname,chl,cam_id,ip,port,userName,password,type){
	var target = $('#'+type+' .'+esee_id+'');
	$("<li><span cam_id = "+cam_id+" class=\"file\" onclick=\"open_close_chl('"+cam_id+"','"+esee_id+"','"+chl+"','"+ip+"','"+port+"','"+userName+"','"+password+"','2');\">"+chlname+"</span></li>").appendTo(target);
	}
//添加区域
function addarea(name,area_id,pid){
	var branches = $('<li id = "'+area_id+'"><span class="area" ><span  class ="modify" onclick = "modify_area_link(this);"></span><span  class="del" onclick = "del_area_link('+area_id+');"></span><a class="area_name">'+name+'</a></span><ul></ul></li>').appendTo(pid == 0 ? $('#add_area_tree') : $('#'+pid+' ul'));
	$('ul.filetree').treeview({add:branches});
	var obj = [];
	obj.push(name);
	obj.push(area_id);
	obj.push(pid);
	branches.find('span.modify').data('data',obj);
	
}
function addareaoption(id,name,level){
	$('<option value="'+id+'" level="'+level+'">'+name+'</option>').appendTo('select');
}*/
/********devInarea.php*******/
/*function Customdev(dev_name,dev_id,area_id){
	if(area_id == 'dev'){
		var target = $('#dev');
	}else{
		var target = $('#'+area_id+'')
	}
	var branches = $('<li class="closed" id = "'+dev_id+'"><span class="folder" ><span onClick="beMoveNode(this);" class="node" >'+dev_name+'</span></span><ul id="folder21"></ul></li>').appendTo(target);
	var obj = [];
	obj.push(dev_name);
	obj.push(dev_id);
	branches.data('data',obj);
}
function Customchl(chl_name,dev_id){
	var target = $('li[id="'+dev_id+'"] #folder21');	
	$('<li><span class="file">'+chl_name+'</span></li>').appendTo(target);
}
function Customarea(area_name,area_id,pid){
	if(pid == '0'){
		var target = $('#area');
	}else{ a
		var target = $('#'+pid+'');a
	}
	var branches = $('<li><span class="area"><span class="target" onClick = "beMoveNode(this);">'+area_name+'</span></span><ul id="'+area_id+'"></ul></li>').appendTo(target);
	var obj = [];
	obj.push(area_id);
	branches.data('data',obj);
	}
//分组设置
function group_add_cam(name,data,action){
	var target = action ? $('ul.'+data[0]+'') : $('.group_manage_right_bottom ul');
	var obj = $('<li><input type="checkbox"/><span class="file">'+name+'</span></li>').appendTo(target);
	obj.data('data',data);
}
function group_cam_option(group_id,remarks,name,cam_info){
	var obj = $('<option value="'+group_id+'"remark="'+remarks+'">'+name+'</option>').appendTo($('#group'));
	obj.data('data',cam_info);	
}*/			
//jq treeview框架,不要动	  
(function($) {
	var CLASSES = $.treeview.classes;
	var proxied = $.fn.treeview;
	$.fn.treeview = function(settings) {
		settings = $.extend({}, settings);
		if (settings.add) {
			return this.trigger("add", [settings.add]);
		}
		if (settings.remove) {
			return this.trigger("remove", [settings.remove]);
		}
		return proxied.apply(this, arguments).bind("add", function(event, branches) {
			$(branches).prev()
				.removeClass(CLASSES.last)
				.removeClass(CLASSES.lastCollapsable)
				.removeClass(CLASSES.lastExpandable)
			.find(">.hitarea")
				.removeClass(CLASSES.lastCollapsableHitarea)
				.removeClass(CLASSES.lastExpandableHitarea);
			$(branches).find("li").andSelf().prepareBranches(settings).applyClasses(settings, $(this).data("toggler"));
		}).bind("remove", function(event, branches) {
			var prev = $(branches).prev();
			var parent = $(branches).parent();
			$(branches).remove();
			prev.filter(":last-child").addClass(CLASSES.last)
				.filter("." + CLASSES.expandable).replaceClass(CLASSES.last, CLASSES.lastExpandable).end()
				.find(">.hitarea").replaceClass(CLASSES.expandableHitarea, CLASSES.lastExpandableHitarea).end()
				.filter("." + CLASSES.collapsable).replaceClass(CLASSES.last, CLASSES.lastCollapsable).end()
				.find(">.hitarea").replaceClass(CLASSES.collapsableHitarea, CLASSES.lastCollapsableHitarea);
			//当li删除完之后 他会继续删除li的父级ul然后继续初始化UI. 影响下一次添加 li
			/*if (parent.is(":not(:has(>))") && parent[0] != this) { 
				parent.parent().removeClass(CLASSES.collapsable).removeClass(CLASSES.expandable)
				parent.siblings(".hitarea").andSelf().remove();
			}*/
		});
	};
	
})(jQuery);