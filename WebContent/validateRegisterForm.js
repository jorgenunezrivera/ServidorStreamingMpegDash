/**
 * Function to validate user registration form
 */

var valid = false;

		function validateRegisterForm()
		{	
			var name=document.getElementById("userName").value;
			var pass = document.getElementById("userPass").value;
			var passConfirm = document.getElementById("userPassConfirm").value;
			var el = document.getElementById("feedback");
		    if(pass != passConfirm)
		    {
		        el.innerHTML = "Las contraseñas no coinciden";		            
		        return false;
		    }
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