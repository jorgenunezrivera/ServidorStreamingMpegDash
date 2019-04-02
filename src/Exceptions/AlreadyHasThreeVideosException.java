package Exceptions;

public class AlreadyHasThreeVideosException extends Exception {
	private static final long serialVersionUID = 1L;
	String name;
	
	public AlreadyHasThreeVideosException(String name) {
		this.name=name;
	}
}
