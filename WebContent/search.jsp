<%@ page language="java" contentType="text/html; charset=UTF-8"
    pageEncoding="UTF-8"%>
<!doctype html>
<html lang="es">
<head>
	<meta charset="UTF-8">
    <title>Servidor Streaming MPEG DASH</title>
   <!--  <script src=validateSearch.js></script> -->
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
			<h2>Buscar videos</h2>
			<p>Esta página le permitirá buscar videos entre los videos publicos de los usuarios.</p>					    
		</div>
		<div id="formDiv">
			<h3>Buscar </h3>
			<form action='SearchResult.jsp' method='post' >
				<p>Buscar</p><input type='text' name='search' id='search' value=''> <br>
				<input type='submit' value='Buscar' onClick='return validateFileUpload();'> <br>
			</form>
		</div>
		<div id="feedback">
		</div>
		        <div id="notification">
		</div>
		
	</div>
</div>
</body>
</html>