package Exceptions;

public class CantCreateUserDirException extends Exception {
String name;
	
	public CantCreateUserDirException(String name) {
		this.name=name;
	}
}
