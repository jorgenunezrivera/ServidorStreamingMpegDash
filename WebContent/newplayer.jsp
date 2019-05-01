<%@ page language="java" contentType="text/html; charset=UTF-8"
    pageEncoding="UTF-8"%>
<!DOCTYPE html>
<html>
<head>
	<meta charset="UTF-8">
	<title>Video.JS Player</title>
	
	
	<link rel="stylesheet" type="text/css" href="style.css">
	<style>
	    video {
	       width: 900px;
	       height: 507px;
	    }
	    body{
	    	min-height:700px;
	    }
	</style>
</head>
<body>
   <div id="container">
   	   <%@include file="newheader.jsp"%>	
	   <div id="mainDiv">
	   	   <div id="infoDiv">
	   	   		<h1>Reproductor MPEG DASH</h1>
	   	   		<p> Reproduciendo el archivo ${param.fileName}</p>
	   	   </div> 
		   <div id="videoDiv">
		       <video id='dash-video' class='video-js' controls preload='auto'  controls>
		       	<source src="${param.fileName}" type="application/dash+xml">
		       </video>
		       <script src='https://vjs.zencdn.net/7.5.4/video.js'></script>	<!-- video.js -->
	<script src='videojs-dash.js'></script>	<!-- video.js -->
	<script src="https://cdn.dashjs.org/latest/dash.all.min.js"></script> <!-- dash.all.min.js -->
	<script>
	var player = videojs('dash-video');
	player.play();
	</script> 
		   </div>
	   </div>
   </div>
</body>
</html>