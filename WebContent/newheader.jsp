<%@ page language="java" contentType="text/html; charset=UTF-8"
    pageEncoding="UTF-8"%>
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<title>Insert title here</title>
</head>
<body>
<div id="headerDiv">
<h1>Servidor de video MPEG-DASH</h1>		
	</div>
	<div id="menuDiv">
		<% if((String)session.getAttribute("userName") != null){ %>
		<div id="menu">
			<div><a href="/ServidorMpegDashJorge/upload.jsp">Upload</a></div>
			<div><a href="/ServidorMpegDashJorge/myvideos.jsp">Mis videos</a></div>
			<div>Editar perfil</div>
			<div><a href="/ServidorMpegDashJorge/logout.jsp">Cerrar sesion</a></div>		
		</div>
		<% } %>	
	</div>
</body>
</html>