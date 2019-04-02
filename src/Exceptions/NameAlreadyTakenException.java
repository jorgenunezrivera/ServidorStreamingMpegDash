package Exceptions;



public class NameAlreadyTakenException extends Exception {
	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;
	String name;
	
	public NameAlreadyTakenException(String name) {
		this.name=name;
	}
	
}
