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
			<h2>Prueba nuestro servidor de streaming de video MPEG-DASH</h2>
			<p>Con este sencillo servicio podrás subir videos de menos de 4 MB y obtener un link para reproducirlo con la tecnología MPEG-DASH</p>
			<p>Esta tecnologí­a permite emplear distintas calidades de video segun la velocidad de la conexión de la persona que lo ve.<br>Esto permite reproducir el video sin cortes y sin necesidad de buffering.</p>
		    <p><a href="https://en.wikipedia.org/wiki/Dynamic_Adaptive_Streaming_over_HTTP">Saber más sobre la tecnologí­a MPEG-DASH</a>		    
		</div>
		<div id="formDiv">
			<h3>¡Prueba el servicio!Carga un archivo al servidor </h3>
			<form action='MPDServer/upload' method='post' enctype='multipart/form-data'>
				<input type='file' name='videoFile' id='videoFile'> <br>
			    <input type='submit' value='Aceptar' onClick='return validateFileUpload();'> <br>
			</form>
		</div>
		<div id="waitDiv" style="display:none;">
			<p>	El archivo se ha cargado con Éxito. Espere un instante mientras este es convertido a formato MPEG-DASH </p>	
		</div>
		<div id="feedback">
		</div>
	</div>
</div>
</body>
</html>