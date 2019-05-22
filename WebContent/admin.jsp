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
				String process=Modelo.getInstance().getQueueProcess();
				if(!process.equals("")){
					int lastSlash = process.lastIndexOf('/');
					String processShort = process.substring(lastSlash);
					int lastDot = processShort.lastIndexOf('.');
					String processBdName = processShort.substring(1, lastDot);
					processBdName += "-DASH/";
					String username = process.substring(0, lastSlash);
					int preLastSlash = username.lastIndexOf('/');
					username = username.substring(preLastSlash + 1);
    			%>	<h4> Procesando ahora:</h4>
                <div class="processContainer">
				<h3><%= processShort %></h3>
				<div>
					<%= Modelo.getInstance().obtenerInfoVideo(processBdName,username) %>
				</div>
            </div>
			 <%}
				if(queue.length>0){
					 %>
					 <div id="queueDiv">
				<h4> A la cola: </h4>	
			<% for(String video : queue){				
					int lastSlash = video.lastIndexOf('/');
					String videoShort = video.substring(lastSlash);
					int lastDot = videoShort.lastIndexOf('.');
					String videoBdName = videoShort.substring(1, lastDot);
					videoBdName += "-DASH/";
					String username = video.substring(0, lastSlash);
					int preLastSlash = username.lastIndexOf('/');
					username = username.substring(preLastSlash + 1);
					%>
				<div class="processContainer">
					<h3><%= videoShort %></h3>
					<div>
						<%=Modelo.getInstance().obtenerInfoVideo(videoBdName, username)%>					
					</div>					
				</div>
			<% } %>
			 <% 
			}%>
		</div>
		<div id="waitDiv" style="display:none;">
			<p>	El archivo se ha cargado con Éxito. Espere un instante mientras este es convertido a formato MPEG-DASH </p>	
		</div>
		<div id="feedback">
		</div>
		        <div id="notification">
		</div>
		
	</div>
</div>
</body>
</html>