package Exceptions;

public class VideoDoesntExistException extends Exception {
/**
	 * 
	 */
	private static final long serialVersionUID = 1L;
String username,filename;
	
	public VideoDoesntExistException(String username,String filename) {
		this.username=username;
		this.filename=filename;
	}

	

}
