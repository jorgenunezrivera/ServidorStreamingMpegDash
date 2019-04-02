package Exceptions;

public class UserDoesntExistException extends Exception {
/**
	 * 
	 */
	private static final long serialVersionUID = 1L;
String name;
	
	public UserDoesntExistException(String name) {
		this.name=name;
	}
}
