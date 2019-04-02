<%@ page language="java" contentType="text/html; charset=UTF-8"
    pageEncoding="UTF-8"%>
<!DOCTYPE html>
<html>
<head>
	<meta charset="UTF-8">
	<title>JS Dash Player</title>
	<link rel="stylesheet" type="text/css" href="style.css">
	<style>
		body{
			min-heigh:186;
		}
	</style>	
</head>
<body>
   <div id="container">
   		<%@include file="newheader.jsp"%>	
	   <div id="mainDiv">
	   	   <div id="infoDiv">
	   	   		<h1>El archivo se ha convertido con exito</h1>
	   	   		<p> El archivo ${param.originalFileName} se ha convertido con exito a formato MPEG-DASH</p>
	   	   		<p>Utilice este enlace para reproducirlo: </p>
	   	   		<a href="player.jsp?fileName=users/${param.streamFileName}" >Reproducir</a>
	   	   </div> 		   
	   </div>
   </div>
</body>
</html>