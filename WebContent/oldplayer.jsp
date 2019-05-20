<%@ page language="java" contentType="text/html; charset=UTF-8"
    pageEncoding="UTF-8"%>
<!DOCTYPE html>
<html>
<head>
	<meta charset="UTF-8">
	<title>JS Dash Player</title>
	<script src="https://cdn.dashjs.org/latest/dash.all.min.js"></script> <!-- dash.all.min.js -->
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
		       <video data-dashjs-player src="${param.fileName}" type="application/dash+xml" controls></video>
		   </div>
	   </div>
   </div>
</body>
</html>