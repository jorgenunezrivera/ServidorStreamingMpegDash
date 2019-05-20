<%@ page language="java" contentType="text/html; charset=UTF-8"
    pageEncoding="UTF-8"%>
<%@ page import="DashJorge.Modelo" %>
<%@ page import="DashJorge.ProcessQueue" %>
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
			<h2>Administrar</h2>
			<p>Aqui podrás ver la cola de procesos de video.</p>					    
		</div>
		<div id="videosDiv">
			<%
			String[] queue=Modelo.getInstance().getQueue();
			for(String video : queue){				
			%>
				<div class="processContainer">
					<p><%= video %></p>
					<div>
						<%=Modelo.getInstance().obtenerInfoVideo(video, userName)%>					
					</div>					
				</div>
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