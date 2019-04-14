package Exceptions;

public class CantCreateUserException extends Exception {
		String name;
	public CantCreateUserException(String name) {
		this.name=name;
	}
	
}
