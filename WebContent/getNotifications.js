function getNotifications() {
	var xhttp = new XMLHttpRequest();
	xhttp.onreadystatechange = function() {
		if(this.readyState==4 && this.status==200) {
			document.getElementById("notification").innerHTML = this.responseText;
			document.getElementById("notification").style.display = "block"
			console.debug("Reciving notifications");
			console.debug(this.responseText);
		}else if(this.readyState==4  && this.status==204){
			document.getElementById("notification").innerHTML = "";
			document.getElementById("notification").style.display = "none";
		}else if(this.readyState==4) {
			document.getElementById("notification").innerHTML = this.responseText;
			document.getElementById("notification").style.display = "block";
			console.debug("Reciving error notifications");
		}
	};
	xhttp.open("GET","/ServidorMpegDashJorge/MPDServer/getNotifications",true);
	xhttp.send();
	console.debug("Checking for notifications");
}

function checkNotifications(){
	setInterval(getNotifications,10000);
	document.getElementById("notification").style.display = "none";
	console.debug("Activated notification system");
}