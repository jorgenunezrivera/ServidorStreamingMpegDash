package DashJorge;

import java.io.File;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import Exceptions.CantRegisterVideoException;
import ffmpeg_jni.VideoDash;

public class ProcessQueue implements Runnable {
	List<String> queue;
	String process;

	List<Notification> notifications;
	int lastDot;
	
	@Override
	public void run() {
		queue=new ArrayList<String>();
		 notifications=new ArrayList<Notification>();
		while(true) {
			if(queue.isEmpty()) {
				process="";
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
				try {
					VideoDash.videoDash(process,dirFullName);
				}catch (Exception e) {
					notifications.add(new Notification(UserName,"Ha habido un error pocesando el video : "+ dirName+ " porque "+e.getMessage()));
					continue;
				}
				
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
		        		notifications.add(new Notification(UserName,"El video ha sido procesado pero no puede ser registrado: "+ dirName+ " porque "+e.getMessage()));
		        		continue;
					}
		        }
		        try {
					Modelo.getInstance().registrarVideoCargado(dirName, UserName,duration);
					notifications.add(new Notification(UserName,"Video procesado con e xito :"+ dirName));
				} catch (CantRegisterVideoException e) {
					// TODO Auto-generated catch block
					notifications.add(new Notification(UserName,"El video ha sido procesado pero no puede ser registrado: "+ dirName+ " porque "+e.getMessage()));
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
	
	public String getProcess() {
		return process;
	}
	
	public List<String> getNotifications(String userName) {
		List<String> q=new ArrayList<String>();
		Iterator<Notification> i = notifications.iterator(); 
		while(i.hasNext()) {
			Notification notification=i.next();
			if(notification.userName.equals(userName)) {
				q.add(notification.notificationText);
				i.remove();
			}			
		}		
		return q;
	}
	
	

}
