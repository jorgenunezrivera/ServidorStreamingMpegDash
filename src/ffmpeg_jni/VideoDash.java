package ffmpeg_jni;



import java.io.IOException;
import java.io.InputStream;
import java.util.Properties;


public class VideoDash {
	static VideoDash instance=null;
	Properties properties;
	String libdir;
	
	VideoDash(){
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
	
	public static VideoDash getInstance()  {
		if(instance==null) {
			instance= new VideoDash();
		}else {
			
		}
		return instance;
}
	
private native int getVideoDash(String filename,String outDir); 

	public static int videoDash(String filename,String outDir) {
		int result = new VideoDash().getVideoDash(filename,outDir);
		return result;
}	
	
}
