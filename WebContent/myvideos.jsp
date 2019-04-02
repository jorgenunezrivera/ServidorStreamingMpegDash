<%@ page language="java" contentType="text/html; charset=UTF-8"
    pageEncoding="UTF-8"%>
<%@ page import="DashJorge.Modelo" %>
<!doctype html>
<html lang="es">
<head>
	<meta charset="UTF-8">
    <title>Servidor Streaming MPEG DASH</title>
    <script src=validateUpload.js></script>
    <link rel="stylesheet" type="text/css" href="style.css">
    <style>
    	body{
    		min-height:500px;
    	}
    </style>
</head>
<body>
<%String userName=(String)session.getAttribute("userName"); 
if(userName==null)response.sendRedirect("/ServidorMpegDashJorge/index.jsp"); %>
<div id=container>
	<%@include file="newheader.jsp"%>	
	<div id="mainDiv">
		
		<div id="infoDiv">
			<h2>Mis videos</h2>
			<p>Aqui podrás ver tus videos. Recuerda que solo puedes tener tres videos y que tus videos se borrarán automáticamente a las 24 horas</p>					    
		</div>
		<div id="videosDiv">
			<%Modelo modelo=Modelo.getInstance();
			String[] videos=modelo.obtenerVideosUsuario(userName);
			for(String video : videos){
			%>
				<div><p><%= video %></p><a href="player.jsp?fileName=users/<%= userName %>/<%= video%>/stream.mpd">Reproducir</a>><a href="MPDServer/delete?fileName=<%= video%>&userName=<%=userName%>">Borrar</a></div>
			<% } %>
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