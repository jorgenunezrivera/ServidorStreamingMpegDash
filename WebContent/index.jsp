<%@ page language="java" contentType="text/html; charset=UTF-8"
    pageEncoding="UTF-8"%>
<!doctype html>
<html lang="es">
<head>
	<meta charset="UTF-8">
    <title>Servidor Streaming MPEG DASH</title>
    <script src="validateLoginForm.js"></script> 
    <script src="validateRegisterForm.js"></script>
    <link rel="stylesheet" type="text/css" href="style.css">
    <style>
    	body{
    		min-height:500px;
    	}
    </style>
</head>
<body>
<% if(session.getAttribute("userName")!=null)response.sendRedirect("/ServidorMpegDashJorge/upload.jsp"); %>
<div id=container>
	<%@include file="newheader.jsp"%>	
	<div id="mainDiv">
		
		<div id="infoDiv">
			<h2>Bienvenido nuestro servidor de streaming de video MPEG-DASH</h2>
			<p>Con este sencillo servicio podrás subir videos de menos de 4 MB y obtener un link para reproducirlo con la tecnología MPEG-DASH</p>
			<p>Esta tecnologí­a permite emplear distintas calidades de video segun la velocidad de la conexión de la persona que lo ve.<br>Esto permite reproducir el video sin cortes y sin necesidad de buffering.</p>
		    <p><a href="https://en.wikipedia.org/wiki/Dynamic_Adaptive_Streaming_over_HTTP">Saber más sobre la tecnologí­a MPEG-DASH</a>		    
		    <p>Registrate para probar nuestro servicio, o inicia sesión si ya eres usuario</p>
		</div>
		<div id="registerDiv">
			<div id="formDiv">
			<h3>Rellena estos campos para registrarte.</h3>
			<form action='MPDServer/register' method='post' >
				<p>Nombre de usuario</p><input type='text' name='userName' id='userName' required> 
				<p>Contraseña</p><input type='password' id='userPass' name='userPass' required>
				<p>Confirmar contraseña</p><input type='password' id='userPassConfirm' name='userPassConf' required>
				<p>Dirección de correo</p><input type='email' name='emailAddr' id='emailAddr' required>
				<p></p><input type='submit' value='Registrarse' onClick='return validateRegisterForm();'> 
				<!-- Escribir función de validación -->
			</form>
			</div>
		</div>
		<div id="loginDiv">
			<div id="formDiv">
				<h3>Introduce tus credenciales para iniciar sesión.</h3>
				<form action='MPDServer/login' method='post' >
					<p>Nombre de usuario</p><input type='text' name='userName' id='userName' required> 
					<p>Contraseña</p><input type='password' id='userPass' name='userPass' required>
					<p></p><input type='submit' value='Iniciar sesión' onClick='return validateLoginForm();'> 
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
