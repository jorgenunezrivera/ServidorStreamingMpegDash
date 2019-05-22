<%@ page language="java" contentType="text/html; charset=UTF-8"
    pageEncoding="UTF-8"%>
<!doctype html>
<html lang="es">
<head>
	<meta charset="UTF-8">
    <title>Servidor Streaming MPEG DASH</title>
    <script src=validateUpload.js></script>
    <link rel="stylesheet" type="text/css" href="style.css">
    <link rel="icon" href="favicon.ico" type="image/x-icon">
    <style>
    	body{
    		min-height:500px;
    	}
    </style>
</head>
<body>
<% if(session.getAttribute("userName")==null)response.sendRedirect("/ServidorMpegDashJorge/index.jsp"); %>
<div id=container>
	<%@include file="newheader.jsp"%>	
	<div id="mainDiv">
		
		<div id="infoDiv">
			<h2>Archivo cargado con éxito</h2>
			<p>El archivo ha sido cargado y esta siendo procesado</p>				
	</div>
	<div id="feedback">
		</div>
		        <div id="notification">
		</div>
		
</div>
</body>
</html>