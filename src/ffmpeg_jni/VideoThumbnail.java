package ffmpeg_jni;

public class VideoThumbnail {
	static {
			String property = System.getProperty("java.library.path");
			System.setProperty("java.library.path",property + ":/usr/local/lib");			
			//System.loadLibrary("VideoThumbnail");
			System.load("/home/brad/ServidorStreamingMpegDash/ffmpeg_jni/jni/libVideoDash.so");//provisional para no instalar cada vez
	   }
	private native int getVideoThumb(String fileName); 
	
	public static int extractVideoThumbnail(String fileName) {
		return new VideoThumbnail().getVideoThumb(fileName);
		
	}
	
	public static void main(String args[]) {
	    int result =extractVideoThumbnail("/home/brad/Descargas/prueba.mp4");
	    System.out.println("result: "+result);
	}
}
