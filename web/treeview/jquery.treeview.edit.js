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