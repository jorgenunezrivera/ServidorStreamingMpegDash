/**
 * Function to validate user login form
 */

var valid = false;

		function validateLoginForm()
		{	
			var name=document.getElementById("userName").value;
			var pass = document.getElementById("userPass").value;
			var el = document.getElementById("feedback");
		    
		    if( /[^a-zA-Z0-9\_]/.test( name ) ) {
		    	el.innerHTML = "El nombre de usuario solo puede contener letras numeros y '_'";		            
		        return false;
		    }
		    if( /[^a-zA-Z0-9\_]/.test( pass ) ) {
		    	el.innerHTML = "La contraseña solo puede contener letras numeros y '_'";		            
		        return false;
		    }
		    return true;
		}