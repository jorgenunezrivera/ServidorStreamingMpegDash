package ffmpeg_jni;



import java.io.IOException;
import java.io.InputStream;
import java.util.Properties;


public class VideoDash {
	static {
		Properties properties;
		String libdir;
		properties=new Properties();
		InputStream input = VideoDash.class.getResourceAsStream("jni.properties");
		try {
			properties.load(input);
		} catch (IOException e) {
			System.err.println("Se ha producido un error leyendo el fichero de configuracion ");
		}
		libdir=properties.getProperty("libdir");		
		System.load(libdir+"libVideoDash.so");
	}
	
	
	
	VideoDash(){
		
	}
	
	

	
private native int getVideoDash(String filename,String outDir); 
private native int getVideoMp4(String filename,String outDir,int streamIndex);
private native String getVideoInfo(String filename);

public static int videoDash(String filename,String outDir) {
	int result = new VideoDash().getVideoDash(filename,outDir);
	return result;
}	
public static int videoMp4(String filename,String outDir, int streamIndex) {
	int result = new VideoDash().getVideoMp4(filename,outDir,streamIndex);
	return result;
}
public static String videoInfo(String filename) {
	return new VideoDash().getVideoInfo(filename);
}
	
	
}
