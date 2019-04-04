<%@ page language="java" contentType="text/html; charset=UTF-8"
    pageEncoding="UTF-8"%>
<!doctype html>
<html lang="es">
<head>
	<meta charset="UTF-8">
    <title>Servidor Streaming MPEG DASH</title>
    <script src="validateNewPass.js"></script>
    <link rel="stylesheet" type="text/css" href="style.css">
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
			<h2>Editar perfil</h2>
			<p>Cambiar contraseña</p>			
		</div>
		<div id="newPasswordDiv">
			<div id="formDiv">
			<h3>Rellena estos campos para cambiar de contraseña.</h3>
			<form action='MPDServer/changepass' method='post' >
				<p>Contraseña antigua</p><input type='password' name='userPass' id='userPass' required> 
				<p>Contraseña</p><input type='password' id='userNewPass' name='userNewPass' required>
				<p>Confirmar contraseña</p><input type='password' id='userNewPassConfirm' name='userNewPassConfirm' required>
				<p></p><input type='submit' value='Cambiar contraseña' onClick='return validateNewPass();'> 
				<!-- Escribir función de validación -->
			</form>
			</div>
		</div>
		<div id="feedback">
		</div>
	</div>
</div>
</body>
</html>
