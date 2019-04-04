package DashJorge;
import javax.mail.*;
import javax.mail.internet.InternetAddress;
import javax.mail.internet.MimeMessage;
import javax.naming.Context;
import javax.naming.InitialContext;
import javax.naming.NamingException;
import javax.sql.DataSource;

import com.sun.mail.smtp.SMTPTransport;

import Exceptions.AlreadyHasThreeVideosException;
import Exceptions.NameAlreadyTakenException;
import Exceptions.UserDoesntExistException;
import Exceptions.VideoDoesntExistException;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.KeySpec;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Timestamp;
import java.util.Calendar;
import java.util.Properties;

import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

import org.apache.commons.codec.DecoderException;
import org.apache.commons.codec.binary.Hex;


public class Modelo {
	
	private static Modelo modelo_instance = null;
	private Connection con;
	private Properties serverProperties;
	TimedClean clean;
	private Modelo() {
		Context initCtx; 
		Context envCtx;
		try {
			initCtx = new InitialContext();
			envCtx = (Context) initCtx.lookup("java:comp/env");
			DataSource ds = (DataSource)envCtx.lookup("jdbc/mariadb");
			con=ds.getConnection();
		} catch (NamingException e1) {
			System.err.println("Imposible conectar a mariadb: nombre de recurso incorrecto");
			e1.printStackTrace();
		} catch (SQLException e) {
			System.err.println("Imposible conectar a mariadb");
			e.printStackTrace();
		}
		serverProperties=new Properties();
		InputStream input = Modelo.class.getResourceAsStream("servidor.properties");
		try {
			serverProperties.load(input);
		} catch (IOException e) {
			System.err.println("Se ha producido un error leyendo el fichero de configuracion (del servidor mysql)");
			
		}
		clean = new TimedClean(this);
	}
	
	public static Modelo getInstance() {
			if(modelo_instance==null) {
				modelo_instance= new Modelo();
			}
			return modelo_instance;
	}
	
	//USUARIO
	
	public void nuevoUsuario(String nombre,String pass,String mailAddr) throws NameAlreadyTakenException{
		try{
			PreparedStatement checkStatement = con.prepareStatement("SELECT username FROM user WHERE username = ?");
			checkStatement.setString(1, nombre);
			checkStatement.execute();
			if(	checkStatement.getResultSet().next()) {
				throw new NameAlreadyTakenException(nombre);
			}
		} catch (SQLException e) {
			System.err.println(e.getMessage());
		}
		try {
			PreparedStatement statement = con.prepareStatement("INSERT INTO user VALUES (?,?,?,FALSE)");
			statement.setString(1, nombre);
			statement.setString(2, generateHash(pass));
			statement.setString(3, mailAddr);
			statement.execute();
			File dir = new File(serverProperties.getProperty("usersdir")+nombre); 
			boolean dirCreated= dir.mkdirs();
			int updated=statement.getUpdateCount();
			//ENVIAR EMAIL DE CONFIRMACION Y GENERAR TOKEN
			//Controlar que sean correctos
			//CONTROLAR QUE LA TRANSACION SEA COMPLETA Y SI NO DESHACERLO TODO
			sendConfirmationMail(nombre,mailAddr);
		}catch (SQLException e) {
			System.err.println(e.getMessage());
		}
			/*if(updated==0) {
				throw new SQLException();
			}*/		
	}
	
	public void borrarUsuario(String nombre) throws UserDoesntExistException {
		try{
			PreparedStatement statement = con.prepareStatement("DELETE FROM user WHERE username = ?");
			statement.setString(1, nombre);
			statement.execute();
			
			int updated=statement.getUpdateCount();
			if(updated>0) {
				File dir = new File(serverProperties.getProperty("usersdir")+nombre);
				recursiveDelete(dir);
				
			}else {
				throw new UserDoesntExistException(nombre);
			}
		} catch (SQLException e) {
			System.err.println(e.getMessage());
		}		
	}
		public void editarPerfil() {	//Por ahora solo seria cambiar de contraseña, y borrar perfil.
		
	}
	
	protected boolean verificarEmail(String nombre,String token) {
		if (checkHash(nombre,token)) {
			try {
				PreparedStatement statement = con.prepareStatement("UPDATE user SET verifieduser=TRUE WHERE username= ?");
				statement.setString(1, nombre);
				statement.execute();
				return true;
			}catch (SQLException e) {
				System.err.println(e.getMessage());
				return false;	//DEBERIA LANZAR UNA EXCEPCION
			}
		}
		return false;	
	}
	
	public boolean autenticarUsuario(String nombre, String pass) {
		try {
			PreparedStatement statement = con.prepareStatement("SELECT password FROM user WHERE username = ?");
			statement.setString(1, nombre);
			statement.execute();
			ResultSet resultado = statement.getResultSet();
			if(resultado.next()) {
				String hashRealPass=resultado.getString(1);
				return checkHash(pass,hashRealPass);
			}
			return false;
		} catch (SQLException e) {
			System.err.println("Error al buscar al usuario en la tabla "+e.getMessage());
			return false;
		}
	}
	//VIDEO
	
	public void registrarVideo(String nombreArchivo,String nombrePropietario) throws AlreadyHasThreeVideosException {
		try {
			//LImpiar y comprobar si hay 3
			clean.run();
			PreparedStatement countStatement = con.prepareStatement("SELECT COUNT(*) AS videocount FROM video WHERE owner = ?");
			countStatement.setString(1, nombrePropietario);
			countStatement.execute();
			ResultSet countSet = countStatement.getResultSet();
			countSet.next();
			int count = countSet.getInt("videocount");
			if(count<3) {
				Timestamp now = new Timestamp(Calendar.getInstance().getTimeInMillis()); 
				Timestamp deletetime=new Timestamp(Calendar.getInstance().getTimeInMillis()+1000*60*60*24); 
				PreparedStatement statement = con.prepareStatement("INSERT INTO video VALUES(?,?,?,?,FALSE);");
				statement.setString(1, nombreArchivo);
				statement.setTimestamp(2,now );
				statement.setTimestamp(3,deletetime );
				statement.setString(4, nombrePropietario);
				statement.execute();
				int updated = statement.getUpdateCount();
				if(updated==0) {
					//	throw new cantregistervideoexception
				}
			}else {
				throw new AlreadyHasThreeVideosException(nombrePropietario);
			}
		}catch (SQLException e) {
			System.err.println(e.getMessage());
		}		
	}
	
	public void registrarVideoCargado(String nombreArchivo,String nombrePropietario) {
		try {
			PreparedStatement statement = con.prepareStatement("UPDATE video SET uploaded=TRUE WHERE filename= ? AND owner = ?;");
			statement.setString(1, nombreArchivo);
			statement.setString(2, nombrePropietario);
			statement.execute();
			int updated = statement.getUpdateCount();
			if(updated==0) {
				//throw new cantregistervideoexception
			}
		}catch (SQLException e) {
			System.err.println(e.getMessage());
		}		
	}
	
	public void borrarVideo(String nombreArchivo,String nombrePropietario) {
		boolean fileUploaded=false;
		System.err.println("archivo: "+ nombreArchivo + "propietario:  " + nombrePropietario);
		try {
			PreparedStatement selectStatement = con.prepareStatement("SELECT * FROM video WHERE filename = ? AND owner = ?");
			selectStatement.setString(1, nombreArchivo);
			selectStatement.setString(2, nombrePropietario);
			selectStatement.execute();
			selectStatement.getResultSet().next();
			if(selectStatement.getResultSet().getBoolean(5)) {
				fileUploaded=true;
			}
			PreparedStatement statement = con.prepareStatement("DELETE FROM video WHERE filename = ? AND owner = ?");
			statement.setString(1, nombreArchivo);
			statement.setString(2, nombrePropietario);
			statement.execute();
			int updated = statement.getUpdateCount();
			if(updated==0) {
				System.err.println("archivo: "+ nombreArchivo + "propietario:  " + nombrePropietario+ "video no borrado");
				//throw new cantdeletevideoexception
			}
			if(fileUploaded) {
				File file = new File(serverProperties.getProperty("usersdir")+nombrePropietario+File.separator +nombreArchivo);//Usar separador universal?
				recursiveDelete(file);
			}
		}catch (SQLException e) {
			System.err.println(e.getMessage());
		}		
	}

	public String obtenerVideo(String nombreArchivo,String nombrePropietario) throws VideoDoesntExistException {
		try {
			PreparedStatement statement = con.prepareStatement("SELECT * FROM video WHERE filename = ? AND owner = ?");
			statement.setString(1, nombreArchivo);
			statement.setString(2, nombrePropietario);
			statement.execute();
			ResultSet resultado=statement.getResultSet();
			if(resultado.next()) {
				boolean isUploaded = resultado.getBoolean(5);
				if(isUploaded) {
					return serverProperties.getProperty("usersdir")+nombrePropietario+"/"+resultado.getString(1);
				}
			}else {
				throw new VideoDoesntExistException(nombreArchivo,nombrePropietario);	
			}
			
		}catch (SQLException e) {
			System.err.println(e.getMessage());
		}	
		return "";
	}
	
	public String[] obtenerVideosUsuario(String nombrePropietario) {
		String[] videos=null;
		try {
			PreparedStatement countStatement = con.prepareStatement("SELECT COUNT(*) as rowcount FROM video Where owner = ?");
			countStatement.setString(1, nombrePropietario);
			countStatement.execute();
			ResultSet countRS= countStatement.getResultSet();
			countRS.next();
			int count=countRS.getInt("rowcount");
			videos=new String[count];
			PreparedStatement statement = con.prepareStatement("SELECT * FROM video WHERE owner = ?");
			statement.setString(1, nombrePropietario);
			statement.execute();
			ResultSet resultado=statement.getResultSet();
			int i=0;
			while(resultado.next()) {
				boolean isUploaded = resultado.getBoolean(5);
				if(isUploaded) {
					videos[i]=resultado.getString(1);
					i++;
				}				
			}
		}catch (SQLException e) {
			System.err.println(e.getMessage());
			}	
		return videos;
	}
	

	public int limpiarVideos() {	//Llamado por la clase "Reloj" para limpiar , devuelve el numero de videos borrados 
		int borrados=0;
		try {
			PreparedStatement statement = con.prepareStatement("SELECT * FROM video WHERE deletetime < NOW();");
			statement.execute();
			ResultSet resultSet=statement.getResultSet();
			while(resultSet.next()) {
				String fileName =resultSet.getString(1);
				String ownerName = resultSet.getString(4);
				System.out.println(serverProperties.getProperty("usersdir")+ownerName+"/"+fileName);
				File file = new File(serverProperties.getProperty("usersdir")+ownerName+"/"+fileName);//Usar separador universal?
				if(recursiveDelete(file)) {
					PreparedStatement deleteStatement = con.prepareStatement("DELETE FROM video WHERE filename = (?) AND owner = (?)");
					deleteStatement.setString(1, fileName);
					deleteStatement.setString(2, ownerName);
					deleteStatement.execute();
					borrados+=deleteStatement.getUpdateCount();
				}
			}
		}catch (SQLException e) {
			System.err.println(e.getMessage());
		}		
		return borrados;
	}
	
	
	
	//FUNCIONES AUXILIARES  O DE APOYO
	protected void sendConfirmationMail(String nombre, String mailAddr) {
		  String from = "jorge.nunez.rivera@udc.es";
	      String host = "smtp.office365.com";
	      String password ="DaisAsko2018";
	      String token=generateHash(nombre);
	      Properties properties = System.getProperties();
	      properties.setProperty("mail.smtp.host", host);
	      properties.setProperty("mail.smtp.auth", "true");
	      properties.put("mail.smtp.starttls.enable","true");
	      properties.setProperty("mail.smtp.port","587");
	      //PUERTO 587
	      //TLS
	      Session session=Session.getDefaultInstance(properties);
	      session.setDebug(true);
	      session.setDebugOut(System.err);
	      try {
	    	  MimeMessage message = new MimeMessage(session);
	          message.setFrom(new InternetAddress(from));
	          message.addRecipient(Message.RecipientType.TO, new InternetAddress(mailAddr));
	          message.setSubject("Correo de confirmación");
	          message.setContent("<h1>Confirma tu usuario en ServidorMpegDASH</h1><p>Haz click en el enlace para confirmar tu correo</p><a href='http://126.37.76.188.dynamic.jazztel.es:9524/ServidorMpegDashJorge/MPDServer/confirmMail?userName="+nombre+"&token="+token+"'>Confirmar correo </a>", "text/html" );
	         //TODO: HOST NAME MUST BE READ FROM PROPERTIES FILE
	          // Send message
	         SMTPTransport t=(SMTPTransport)session.getTransport("smtp");
	         try {
	        	 t.connect(host,from, password);
	        	  t.sendMessage(message, message.getAllRecipients());
	         } catch (SendFailedException e) {
	        	 e.printStackTrace();
	        	 System.err.println(e.getMessage());
	         }catch (MessagingException mex) {
	        	 mex.printStackTrace();
	        	 System.err.println(mex.getMessage());
	         }
	         finally {
	        	 t.close();
	         }
	         
	         
	      } catch (MessagingException mex) {
	         mex.printStackTrace();
	         System.err.println(mex.getMessage());
	      }
	}
	
	
	
	private String generateHash(String pass) {
		SecureRandom random = new SecureRandom();
		byte[] salt = new byte[16];
		byte[] hash = null;
		random.nextBytes(salt);
		String saltString=Hex.encodeHexString(salt);
		KeySpec spec = new PBEKeySpec(pass.toCharArray(), salt, 65536, 128);
		try {
			SecretKeyFactory factory = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA1");
			hash = factory.generateSecret(spec).getEncoded();
			
		} catch (NoSuchAlgorithmException e) {
			System.err.println("No existe el algoritmo o el nombre está mal escrito");
		} catch (InvalidKeySpecException e) {
			System.err.println("Especificación no válida, revisar");
		}
		String hashString =Hex.encodeHexString(hash);
		return saltString + hashString;
	}
	
	private boolean checkHash(String pass,String hashString) {
		byte[] salt = new byte[16];
		byte[] realHash = new byte[16];
		byte[] newHash=new byte[16];
		byte[] salthash=new byte[32];
		//char[] salthash=hashString.toCharArray();
		try {
			salthash=Hex.decodeHex(hashString);
		} catch (DecoderException e1) {
			System.err.println("Imposible decodificar el hash"+e1.getMessage());
		}
		for(int i=0;i<16;i++) {
			salt[i]=salthash[i];
		}
		for(int i=16;i<32;i++) {
			realHash[i-16]=salthash[i];
		}
		String saltString=Hex.encodeHexString(salt);
		String realHashString=Hex.encodeHexString(realHash);
		KeySpec spec = new PBEKeySpec(pass.toCharArray(), salt, 65536, 128);
		try {
			SecretKeyFactory factory = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA1");
			newHash = factory.generateSecret(spec).getEncoded();
			
		} catch (NoSuchAlgorithmException e) {
			System.err.println("No existe el algoritmo o el nombre está mal escrito");
		} catch (InvalidKeySpecException e) {
			System.err.println("Especificación no válida, revisar");
		}
		String newHashString=Hex.encodeHexString(newHash);
		String newSaltHashString =saltString+newHashString;		
		return hashString.equals(newSaltHashString);
	}	
	

	private boolean recursiveDelete(File file) {
		if(file.isDirectory()){
			for (File son: file.listFiles()) {
				if(recursiveDelete(son)==false) {
					return false;
				};				
			}
			return file.delete();
		}
		else if(file.isFile()) {
			return file.delete();
		}	
		return false;
	}		
	
	public void close() {
		clean.destroy();
		try {
			con.close();
		} catch (SQLException e) {
			System.err.println("Imposible cerrar la conexion con MariaDB");
			e.printStackTrace();
		}
	}



}
