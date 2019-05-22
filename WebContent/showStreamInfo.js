function showStreams(e){
    console.debug(e);
    var father=document.getElementById(e).parentElement;
    if(document.getElementById(e).style.display==="block"){
        document.getElementById(e).style.display="none";
        father.style.height="234px";
    }else{
		document.getElementById(e).style.display="block";
        father.style.height="434px";
	}
}
