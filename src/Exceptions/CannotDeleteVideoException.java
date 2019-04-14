package Exceptions;

public class CannotDeleteVideoException extends Exception {
String name;
	
	public CannotDeleteVideoException(String name) {
		this.name=name;
	}
}
