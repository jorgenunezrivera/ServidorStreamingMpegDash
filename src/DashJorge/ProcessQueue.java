package DashJorge;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import Exceptions.CantRegisterVideoException;
import ffmpeg_jni.VideoDash;

public class ProcessQueue implements Runnable {
	List<String> queue;
	String process;
	int lastDot;
	
	@Override
	public void run() {
		queue=new ArrayList<String>();
		while(true) {
			if(queue.isEmpty()) {
				try {
					Thread.sleep(500);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}else {
				process=queue.remove(0);
				System.err.println("Procesando: "+process);
				
				int lastSlash = process.lastIndexOf("/");
				String FatherDirName=process.substring(0,lastSlash);
				String fileName=process.substring(lastSlash+1,process.length());
				String dirName;
				lastDot = fileName.lastIndexOf(".");
				dirName=fileName.substring(0,lastDot);
				dirName += "-DASH/";
				
				lastSlash=FatherDirName.lastIndexOf("/");
				String UserName=process.substring(lastSlash+1,FatherDirName.length());
				
				lastDot = process.lastIndexOf(".");
				String dirFullName=process.substring(0,lastDot);
				dirFullName += "-DASH/";
				
				VideoDash.videoDash(process,dirFullName);
				
				File file = new File(process); 
		        file.delete();
		        
		        String info=VideoDash.videoInfo(dirFullName+"stream.mpd");
		        String[] infoParts=info.split(";");
		        String[] streamInfo;
		        int duration =Integer.parseInt(infoParts[0]);
		        int nb_streams=Integer.parseInt(infoParts[1]);
		        int bit_rate,width,height;
		        for(int i=0;i<nb_streams;i++) {
		        	streamInfo=infoParts[i+2].split(",");
		        	bit_rate=Integer.parseInt(streamInfo[0]);
		        	width=Integer.parseInt(streamInfo[1]);
		        	height=Integer.parseInt(streamInfo[2]);
		        	try {
		        		Modelo.getInstance().registrarStreamVideo(dirName,UserName,i,bit_rate,width,height);
		        	} catch (CantRegisterVideoException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
		        }
		        try {
					Modelo.getInstance().registrarVideoCargado(dirName, UserName,duration);
				} catch (CantRegisterVideoException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
		    
			}
		}
		
	}
	
	public void addString(String s) {
		if(queue != null) {
			queue.add(s);
		}
	}
	
	public String[] getQueue() {
		String[] q=new String[queue.size()];
		for(int i=0;i<queue.size();i++) {
			q[i]=queue.get(i);
		}
		return q;
	}
	
	

}
