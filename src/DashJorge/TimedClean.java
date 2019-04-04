package DashJorge;

import java.util.Timer;
import java.util.TimerTask;

import DashJorge.Modelo;

class TimedClean {
	Timer timer;
	TimerTask task;
	Modelo modelo;
    public TimedClean(Modelo modelo) {
    	this.modelo=modelo;
	    timer = new Timer();
	     task = new TimerTask() {
	    
	          
	        @Override
	        public void run()
	        {
	          int borrados=modelo.limpiarVideos();
	          System.out.println("Cleaner ha borrado " + borrados + " videos");
	        }
	        };
	        // Programa la tarea para que funcione cada 15 minutos
	    timer.schedule(task, 0, 900000);
    }
    
    public void run() {
    	task.run();
    	
    }
    
    public void destroy() {
    	task.cancel();
    	timer.cancel();
    }
}