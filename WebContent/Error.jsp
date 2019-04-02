<%@ page language="java" contentType="text/html; charset=UTF-8"
    pageEncoding="UTF-8"%>
<!DOCTYPE html>
<html>
<head>
	<meta charset="UTF-8">
	<title>Error</title>
	<link rel="stylesheet" type="text/css" href="style.css">
	<style>
		body{
			min-heigh:186;
		}
	</style>	
</head>
<body>
   <div id="container">
   		<%@include file="newheader.jsp"%>	
	   <div id="mainDiv">
	   	   <div id="infoDiv">
	   	   		<h1>Se ha producido un error</h1>
	   	   		<p> "${param.message}"</p>
	   	   		<a href="index.jsp" >Atras</a>
	   	   </div> 		   
	   </div>
   </div>
</body>
</html>