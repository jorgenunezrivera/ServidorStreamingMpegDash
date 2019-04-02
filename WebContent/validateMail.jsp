<%@ page language="java" contentType="text/html; charset=UTF-8"
    pageEncoding="UTF-8"%>
<!doctype html>
<html lang="es">
<head>
	<meta charset="UTF-8">
    <title>Servidor Streaming MPEG DASH</title>
    <link rel="stylesheet" type="text/css" href="style.css">
    <style>
    	body{
    		min-height:500px;
    	}
    </style>
</head>
<body>

<div id=container>
	<% if(session.getAttribute("waitingMailConfirmation")==null)response.sendRedirect("/ServidorMpegDashJorge/index.jsp"); %>
	<%@include file="newheader.jsp"%>	
	<div id="mainDiv">
		<div id="infoDiv">
			<h2>Cconfirmar correo electrónico</h2>
			<p>Hemos enviado un email de confirmación .Probablemente este en el correo basura</p>
			<p><a href="MPDServer/ReSendMail">Volver a enviar correo</a>		    
		</div>		
	</div>
</div>
</body>
</html>