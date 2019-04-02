/**
 * Function to validate upload file
 */

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
		        if(file.size>4000000)
		        	{
		        		el.innerHTML="El tamaño máximo son 4MB";
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
		    el.innerHTML="El archivo debe tener como extension .avi , .mp4, .flv";
		    return false;
		    
		}