/**
 * Function to validate upload file
 */
//Comprobar si el usuario "SOY YO" dejarme subir videos grandes que quiero ver la peli de batman desde la cama
var valid = false;

		function validateFileUpload()
		{	
			var input_element=document.getElementById("videoFile");
			var file;
    		var el = document.getElementById("feedback");
		    var fileName = input_element.value;
		    var allowed_extensions = new Array("mp4","avi","flv","3gp");
		    var file_extension = fileName.split('.').pop(); 
		    if (!input_element.files[0]) {
		        el.innerHTML="Por favor elige un archivo";
		        return false;
		    }
		    else {
		        file = input_element.files[0];
		        if(file.size>10*1024*1024)
		        	{
		        		el.innerHTML="El tamaño máximo son 10MB";
			          	return false;
		        	}
		    }
		    for(var i = 0; i < allowed_extensions.length; i++)
		    {
		        if(allowed_extensions[i]==file_extension)
		        {
		            el.innerHTML = "";
		            document.getElementById("waitDiv").style.display="block";
		            return true;
		        }
		    }
		    el.innerHTML="El archivo debe tener como extension .avi , .mp4, .flv  .3gp  ...";
		    return false;
		    
		}