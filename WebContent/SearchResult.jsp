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
String search=(String)request.getParameter("search");
if(userName==null)response.sendRedirect("/ServidorMpegDashJorge/index.jsp"); %>
<div id=container>
	<%@include file="newheader.jsp"%>	
	<div id="mainDiv">
		
		<div id="infoDiv">
			<h2>Resultado de la busqueda</h2>
			<p>Estos son los videos encontrados:</p>					    
		</div>
		<div id="videosDiv">
			<%Modelo modelo=Modelo.getInstance();
			String[] videos=modelo.buscarVideos(search);
			String owner,fileName;
			for(String video : videos){
				int slash=video.indexOf('/');
				owner=video.substring(0,slash);
				fileName=video.substring(slash+1);				
			%>
				<div class="videoInfoContainer">
					<h2><%= video %></h2>
					<div>
						<a href="player.jsp?fileName=users/<%= video%>stream.mpd">
						<img src="users/<%= video%>/pre.jpg"> </img></a></br>
					</div>	
					<div>
                <%String infovideo =  Modelo.getInstance().obtenerInfoVideo(fileName, owner);
						  int streamInfoIndex=infovideo.indexOf("Video");
						  String simpleInfo=infovideo.substring(0,streamInfoIndex);
						  String streamInfo=infovideo.substring(streamInfoIndex);
						%>
                    <%= simpleInfo %>
											
					</div>					
					<a href="player.jsp?fileName=users/<%= video%>stream.mpd"><img class="button" src="play.png"/></a>					
				</div>
			<% } %>
		</div>
		<div id="feedback">
		</div>
		        <div id="notification">
		</div>
		
	</div>
</div>
</body>
</html>