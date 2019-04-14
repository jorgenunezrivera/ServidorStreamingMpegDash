package Exceptions;

public class CantRegisterVideoException extends Exception {
	String name;
	public CantRegisterVideoException(String name) {
		this.name=name;
	}
}
