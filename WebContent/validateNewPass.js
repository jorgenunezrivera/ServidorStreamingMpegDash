/**
 * Function to validate user registration form
 */

var valid = false;

		function validateNewPass()
		{	
			var pass = document.getElementById("userNewPass").value;
			var passConfirm = document.getElementById("userNewPassConfirm").value;
			var el = document.getElementById("feedback");
		    if(pass != passConfirm)
		    {
		        el.innerHTML = "Las contraseñas no coinciden";		            
		        return false;
		    }
		    if( /[^a-zA-Z0-9\_]/.test( pass ) ) {
		    	el.innerHTML = "La contraseña solo puede contener letras numeros y '_'";		            
		        return false;
		    }
		    return true;
		}