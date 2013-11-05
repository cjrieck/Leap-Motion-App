$(function() {
	$(".headerspan").css('line-height', $(".headerdiv").height()+'px');

	$("a").click(function(ev) {
		if ( /\#/.test($(this).prop('href')) ) {
			ev.preventDefault();
			var timeToScroll = 750;
			var anchorLink = $(this).prop('href');
			var splitList = anchorLink.split("#");
			var anchorDest = "#" + splitList[splitList.length - 1];

			if (anchorDest == "#") {
				destPos = 0;
			}
			else {
				var pageSize = $(window).height();
				var offset = (pageSize * .25);
				var destPos = ( $(anchorDest).offset().top ) - offset;
			}

			$(document.body).stop().animate({
				'scrollTop': destPos
			}, timeToScroll);
		}
	});
});