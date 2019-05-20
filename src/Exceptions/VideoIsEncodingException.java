package Exceptions;

public class VideoIsEncodingException extends Exception {
String username,filename;
	
	public VideoIsEncodingException(String username,String filename) {
		this.username=username;
		this.filename=filename;
	}
}
