<%@ page language="java" contentType="text/html; charset=UTF-8"
    pageEncoding="UTF-8"%>
<%@ page import="DashJorge.Modelo" %>
<!doctype html>
<html lang="es">
<head>
	<meta charset="UTF-8">
    <title>Servidor Streaming MPEG DASH</title>
    <script src=showStreamInfo.js></script>
    <link rel="stylesheet" type="text/css" href="style.css">
    <style>
    	body{
    		min-height:500px;
    	}
    </style>
</head>
<body >
<%String userName=(String)session.getAttribute("userName"); 
if(userName==null)response.sendRedirect("/ServidorMpegDashJorge/index.jsp"); %>
<div id=container>
	<%@include file="newheader.jsp"%>	
	<div id="mainDiv">
		
		<div id="infoDiv">
			<h2>Mis videos</h2>
			<p>Aqui podrás ver tus videos.</p>					    
		</div>
		<div id="videosDiv">
			<%Modelo modelo=Modelo.getInstance();
			String[] videos=modelo.obtenerVideosUsuario(userName);
			for(String video : videos){
				
			%>
				<div class="videoContainer">
					<h2><%= video %></h2>
					<div>
						<a href="player.jsp?fileName=users/<%= userName %>/<%= video%>stream.mpd">
						<img src="users/<%= userName %>/<%= video%>/pre.jpg"> </img></a></br>
					</div>	
					<div>
						<%String infovideo = Modelo.getInstance().obtenerInfoVideo(video, userName);
						  int streamInfoIndex=infovideo.indexOf("Video");
						  String simpleInfo=infovideo.substring(0,streamInfoIndex);
						  String streamInfo=infovideo.substring(streamInfoIndex);
						%>
						<p> <%=simpleInfo%> </p>
						
					</div>
					<div>
					<a href="player.jsp?fileName=users/<%= userName %>/<%= video%>stream.mpd"><img class="button" src="play.png"/></a>
					<a href="MPDServer/delete?fileName=<%= video%>&userName=<%=userName%>"><img class="button" src="delete.png"/></a>
					</div>
					<input type='submit' value= 'Ver informacion de los streams' onClick='showStreamInfo()'/>
						<div id="streamInfoDiv" style="display:none;">
							<p>	<%= streamInfo %> </p>	
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