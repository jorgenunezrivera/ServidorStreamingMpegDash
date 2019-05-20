<%@ page language="java" contentType="text/html; charset=UTF-8"
    pageEncoding="UTF-8"%>
<%@ page import="DashJorge.Modelo" %>
<!doctype html>
<html lang="es">
<head>
<meta charset="UTF-8">
<title>Servidor Streaming MPEG DASH</title>

	<%String userName=(String)session.getAttribute("userName");
Modelo modelo=Modelo.getInstance();
String filename=request.getParameter("fileName");
int lastSlash=filename.lastIndexOf("/");

String video=filename.substring(0, lastSlash);
int preSlash=video.lastIndexOf("/");
video=video.substring(preSlash+1);
video+="/";
if(userName==null)response.sendRedirect("/ServidorMpegDashJorge/index.jsp"); %>

<script src=validateUpload.js></script>
<script src="https://reference.dashif.org/dash.js/v2.5.0/dist/dash.all.debug.js"></script> <!-- dash.all.min.js -->
<script src="https://reference.dashif.org/dash.js/v2.5.0/contrib/akamai/controlbar/ControlBar.js"></script>
<script>
    function startVideo() {
        const url = '<%=filename%>';
        var videoElement = document.querySelector(".videoContainer video");

        var player = dashjs.MediaPlayer().create();
        player.initialize(videoElement, url, true);
        var controlbar = new ControlBar(player);
        controlbar.initialize();
    }

</script>
	<style>
	    video {
	       width: 900px;
	       height: 507px;
	    }
	    body{
	    	min-height:700px;
	    }
	</style>
<link rel="stylesheet" type="text/css" href="style.css">
<link rel="stylesheet" type="text/css" href="controlbar.css">
<style>
body {
	min-height: 500px;
}
</style>
</head>
<body>

	<div id=container>
		<%@include file="newheader.jsp"%>
		<div id="mainDiv">
			<div id="infoDiv">
				<h2>Reproductor</h2>
				<p>Aqui podrás ver tus videos y los videos publicos de otros	usuarios.</p>
			</div>
			<div class="dash-video-player col-md-9">
                <div id="videoContainer" class="videoContainer">
                    <video preload="auto"  autoplay="true"></video>
                    <div id="video-caption" style="position: absolute; display: flex; overflow: hidden; pointer-events: none; top: 0px; left: 0px;"></div>
                    <div id="videoController" class="video-controller unselectable">
                        <div id="playPauseBtn" class="btn-play-pause" data-toggle="tooltip" data-placement="bottom" title="" data-original-title="Play/Pause">
                            <span id="iconPlayPause" class="icon-pause"></span>
                        </div>
                        <span id="videoTime" class="time-display">02:24</span>
                        <div id="fullscreenBtn" class="btn-fullscreen control-icon-layout" data-toggle="tooltip" data-placement="bottom" title="" data-original-title="Fullscreen">
                            <span class="icon-fullscreen-enter"></span>
                        </div>
                        <div id="bitrateListBtn" class="btn-bitrate control-icon-layout" data-toggle="tooltip" data-placement="bottom" title="" data-original-title="Bitrate List">
                            <span class="icon-bitrate"></span>
                        </div>
                        <input type="range" id="volumebar" class="volumebar" value="1" min="0" max="1" step=".01">
                        <div id="muteBtn" class="btn-mute control-icon-layout" data-toggle="tooltip" data-placement="bottom" title="" data-original-title="Mute">
                            <span id="iconMute" class="icon-mute-off"></span>
                        </div>
                        <div id="trackSwitchBtn" class="btn-track-switch control-icon-layout hide" data-toggle="tooltip" data-placement="bottom" title="" data-original-title="Track List">
                            <span class="icon-tracks"></span>
                        </div>
                        <div id="captionBtn" class="btn-caption control-icon-layout hide" data-toggle="tooltip" data-placement="bottom" title="" data-original-title="Closed Caption / Subtitles">
                            <span class="icon-caption"></span>
                        </div>
                        <span id="videoDuration" class="duration-display">03:04</span>
                        <div class="seekContainer">
                            <input type="range" id="seekbar" value="0" class="seekbar" min="0" step="0.01" max="184.84">
                        </div>
                    <div id="bitrateMenu" class="menu unselectable menu-item-unselected hide" style="top: 572px; left: 915px; position: fixed;"><div id="video"><div class="menu-sub-menu-title">Video</div><ul id="videoContent" class="video-menu-content"><li id="video-bitrate-listItem_0" class="menu-item-unselected"> Auto Switch</li><li id="video-bitrate-listItem_1" class="menu-item-unselected">116 kbps</li><li id="video-bitrate-listItem_2" class="menu-item-unselected">303 kbps</li><li id="video-bitrate-listItem_3" class="menu-item-unselected">543 kbps</li><li id="video-bitrate-listItem_4" class="menu-item-unselected">760 kbps</li><li id="video-bitrate-listItem_5" class="menu-item-unselected">1247 kbps</li><li id="video-bitrate-listItem_6" class="menu-item-unselected">1388 kbps</li><li id="video-bitrate-listItem_7" class="menu-item-selected">1807 kbps</li></ul></div></div></div>
                </div>
            </div>
			
			<div>
			<%= modelo.obtenerInfoVideo(video, userName)%> 
				
			</div>

		</div>
		<div id="feedback"></div>
	</div>
</body>
</html>