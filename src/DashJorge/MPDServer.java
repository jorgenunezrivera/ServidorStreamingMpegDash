package DashJorge;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.PrintWriter;
import java.nio.file.Paths;
import java.util.List;
import java.util.Properties;
import javax.servlet.ServletException;
import javax.servlet.ServletOutputStream;
import javax.servlet.annotation.MultipartConfig;
import javax.servlet.annotation.WebServlet;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.http.HttpSession;
import javax.servlet.http.Part;

import Exceptions.AlreadyHasThreeVideosException;
import Exceptions.CannotDeleteVideoException;
import Exceptions.CantCreateUserDirException;
import Exceptions.CantCreateUserException;
import Exceptions.CantRegisterVideoException;
import Exceptions.NameAlreadyTakenException;
import Exceptions.UserDoesntExistException;
import ffmpeg_jni.VideoDash;


/**
 * Servlet implementation class MPDServer
 */
@WebServlet("/MPDServer")
@MultipartConfig
public class MPDServer extends HttpServlet {
	private static final long serialVersionUID = 1L;
	private static Modelo modelo;
	private Properties serverProperties;
	
	
	/**
     * Default constructor. 
     */
    public MPDServer() {
        modelo=Modelo.getInstance();        
       serverProperties=new Properties();
    	InputStream input = Modelo.class.getResourceAsStream("servidor.properties");
    	try {
			serverProperties.load(input);
			input.close();
		} catch (IOException e) {
			System.err.println(e.getMessage());
		} finally {
			
		}
		
    }

	/**
	 * @see HttpServlet#doGet(HttpServletRequest request, HttpServletResponse response)
	 *   //get to confirm_mail and delete
	 *   El resto no se admiten
	 */
	protected void doGet(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
		String URI=request.getRequestURI();
		System.out.println(URI);
		response.setStatus(HttpServletResponse.SC_NOT_FOUND              );
		//ServletOutputStream os =response.getOutputStream();
		if(URI.equals("/ServidorMpegDashJorge/MPDServer/confirmMail")) {
			doConfirmEmail(request,response);
		}else if (URI.equals("/ServidorMpegDashJorge/MPDServer/delete")) {
			doDelete(request,response);
		}else if (URI.equals("/ServidorMpegDashJorge/MPDServer/getNotifications")) {
			doGetNotifications(request,response);
		}else if (URI.equals("/ServidorMpegDashJorge/MPDServer/download")) {
			doDownload(request,response);
		}else{			
			response.setStatus(HttpServletResponse.SC_BAD_REQUEST);
			ServletOutputStream os =response.getOutputStream();
			os.println("El recurso "+ URI + " no está disponible para ti");
			return;
		}
	}

	/**
	 * @throws IOException 
	 * @throws ServletException 
	 * @see HttpServlet#doPost(HttpServletRequest request, HttpServletResponse response)
	 * post para: waitupload,register,login,changePass;
	 * si la request es distinta devuelve bad request
	 */
	protected void doPost(HttpServletRequest request, HttpServletResponse response) throws IOException, ServletException {
		String URI=request.getRequestURI();
		System.out.println(URI);
		if(URI.equals("/ServidorMpegDashJorge/MPDServer/upload")) {
			doUpload(request,response);
		}else if(URI.equals("/ServidorMpegDashJorge/MPDServer/register")) {
			doRegister(request,response);
		}else if(URI.equals("/ServidorMpegDashJorge/MPDServer/login")) {
			doLogin(request,response);
		}else if(URI.equals("/ServidorMpegDashJorge/MPDServer/changepass")) {
		doChangePass(request,response);
		}else{
			ServletOutputStream os =response.getOutputStream();
			response.setStatus(HttpServletResponse.SC_BAD_REQUEST);
			os.println("El recurso "+ URI + " no está disponible para ti");
			return;
		}
	}

	/**
	 * 
	 * @param request
	 * @param response
	 * @throws IOException
	 * gestiona la recepción del archivo, su procesado e ingreso en base de datos
	 */
	protected void doUpload(HttpServletRequest request, HttpServletResponse response) throws IOException {
		Part filePart = null;
		String fileName;
		HttpSession session=request.getSession();  
		String username=(String)session.getAttribute("userName");
		String title=(String)request.getParameter("title");
		String description=(String)request.getParameter("description");
		Boolean publicVideo=request.getParameter("publicVideo")!=null;
		//RECIBIR EL ARCHIVO
		try {
			filePart = request.getPart("videoFile");			
		} catch (IOException | ServletException e1) {
			System.err.println("No se puede recibir el archivo");
			e1.printStackTrace();
			response.setStatus(HttpServletResponse.SC_BAD_REQUEST);
			return;
		}
		fileName =filePart.getSubmittedFileName().toLowerCase();// Paths.get(filePart.getSubmittedFileName()).getFileName().toString(); // MSIE fix. ???
		String fileWrite = serverProperties.getProperty("usersdir")+ username+File.separator + fileName;
		//ESCRIBIR EL ARCHIVO
		try {
			filePart.write(fileWrite);
		} catch (IOException e1) {
			System.err.println("No se puede escribir el archivo en el servidor, tal vez ya exista uno con el mismo nombre o el disco este lleno");
			e1.printStackTrace();
			response.setStatus(HttpServletResponse.SC_BAD_REQUEST);
			return;
		}
		int lastDot = fileName.lastIndexOf(".");
		String dirName=fileName.substring(0,lastDot);
		dirName += "-DASH/";
		//AÑADIR A BD
		try {
			modelo.registrarVideo(dirName, username,title,description,publicVideo);
		} catch (AlreadyHasThreeVideosException e2) {
			response.sendRedirect("/ServidorMpegDashJorge/Error.jsp?message=Ya tienes tres videos subidos, borra uno de ellos para subir más");
			return;
		} catch (CantRegisterVideoException e) {
			response.sendRedirect("/ServidorMpegDashJorge/Error.jsp?message=Error: No se ha podido regsitrar el video en la base de datos");
			return;
		}
		 //PROCESAR EL VIDEO	
		lastDot = fileWrite.lastIndexOf(".");
		String dirCompleteName=fileWrite.substring(0,lastDot);
		dirCompleteName += "-DASH/";
		modelo.addToQueue(fileWrite);
		response.sendRedirect("/ServidorMpegDashJorge/uploadcomplete.jsp");		
	}
	
 
	
	//REGISTRAR USUARIO
	protected void doRegister(HttpServletRequest request,HttpServletResponse response)throws IOException, ServletException {
		String userName =(String)request.getParameter("userName");
		String userPass =(String)request.getParameter("userPass");
		String emailAddr=(String)request.getParameter("emailAddr");
		HttpSession session=request.getSession();  
		//session.setAttribute("waitingMailConfirmation",true);
		try{
			modelo.nuevoUsuario(userName, userPass,emailAddr);
			//response.sendRedirect("/ServidorMpegDashJorge/validateMail.jsp");//NO CONFIRMAR MAIL
			session.setAttribute("userName",userName);  
			response.sendRedirect("/ServidorMpegDashJorge/upload.jsp");
		} catch (CantCreateUserDirException | CantCreateUserException e) {
			response.sendRedirect("/ServidorMpegDashJorge/Error.jsp?message=Error; No se ha podido crear el usuario");
		}catch (NameAlreadyTakenException e) {
			response.sendRedirect("/ServidorMpegDashJorge/Error.jsp?message=Ya existe un usuario con ese nombre, elija otro nombre de usuario por favor");
		}
		
		
	}
	
	//INICIAR SESION	
	protected void doLogin(HttpServletRequest request,HttpServletResponse response)throws IOException, ServletException {
		String userName =(String)request.getParameter("userName");
		String userPass =(String)request.getParameter("userPass");
		if (modelo.autenticarUsuario(userName, userPass)) {
			HttpSession session=request.getSession();  
	        session.setAttribute("userName",userName);  
			response.sendRedirect("/ServidorMpegDashJorge/upload.jsp");
		}else {
			response.sendRedirect("/ServidorMpegDashJorge/Error.jsp?message=No se ha podido iniciar sesión, el nombre de usuario o contraseña son incorrectos");
		}
	}
	
	//CONFIRMAR CORREO
	protected void doConfirmEmail(HttpServletRequest request,HttpServletResponse response)throws IOException, ServletException {
		String userName =(String)request.getParameter("userName");
		String token =(String)request.getParameter("token");
		if (modelo.verificarEmail(userName, token)) {
			HttpSession session=request.getSession();  
	        session.setAttribute("userName",userName);
	        session.removeAttribute("WaitingMailConfirmation");
			response.sendRedirect("/ServidorMpegDashJorge/upload.jsp");
		}else {
			response.sendRedirect("/ServidorMpegDashJorge/Error.jsp?message=No se ha podido iniciar sesión, el nombre de usuario o contraseña son incorrectos");
		}
	}
	//BORRAR VIDEO
	protected void doDelete(HttpServletRequest request,HttpServletResponse response) throws IOException {
		HttpSession session=request.getSession();  
		String userName=(String)session.getAttribute("userName");
		if(request.getParameter("userName").equals(userName)) {
			try {
				modelo.borrarVideo((String)request.getParameter("fileName"),userName );
			} catch (CannotDeleteVideoException e) {
				response.sendRedirect("/ServidorMpegDashJorge/Error.jsp?message=Error: No se ha podido borrar el video");
			}
			response.sendRedirect("/ServidorMpegDashJorge/myvideos.jsp");
		}else {
			response.sendRedirect("/ServidorMpegDashJorge/Error.jsp?message=No puedes borrar un video de otro  usuario pillín!!!!!");
		}
	}
	//CAMBIAR CONTRASEÑA
	protected void doChangePass(HttpServletRequest request,HttpServletResponse response)throws IOException {
		HttpSession session=request.getSession();  
		String userName=(String)session.getAttribute("userName");
		String oldPass=(String)request.getParameter("userPass");
		String newPass=(String)request.getParameter("userNewPass");
		if(modelo.autenticarUsuario(userName, oldPass)) {
			try {
				modelo.editarUsuario(userName,newPass);
			} catch (UserDoesntExistException e) {
				response.sendRedirect("/ServidorMpegDashJorge/Error.jsp?message=El usuario no existe");
			}
			response.sendRedirect("/ServidorMpegDashJorge/upload.jsp");
		}else{
			response.sendRedirect("/ServidorMpegDashJorge/Error.jsp?message=La contraseña no es correcta");
		}
	}
	
	protected void doGetNotifications(HttpServletRequest request,HttpServletResponse response)throws IOException {
		PrintWriter out=response.getWriter();
		HttpSession session=request.getSession();  
		String userName=(String)session.getAttribute("userName");
		out.println("<?xml version=\"1.0\" encoding=\"utf-8\"?>" );
		if(userName.equals("")) {
			out.println("<p>No estas iniciado!!</p>");
			response.setStatus(HttpServletResponse.SC_BAD_REQUEST);
			return;
		}
		List<String> notifications=modelo.getNotifications(userName);
		if(notifications.isEmpty()) {
			out.println("<div class='notificationDiv'></div>");
			response.setStatus(HttpServletResponse.SC_NO_CONTENT);
			return;
		}
		for(String notification: notifications ) {
			out.println("<div class='notificationDiv'> <p>"+notification + "</p></div>");
		}
		response.setStatus(HttpServletResponse.SC_OK);
		return;
	}
	
	public void doDownload(HttpServletRequest request,HttpServletResponse response)throws IOException {
		//String userName=(String)session.getAttribute("userName");
		String ownerName=(String)request.getParameter("userName");
		String fileName=(String)request.getParameter("fileName");
		int numStreams=Integer.parseInt(request.getParameter("numStreams"));
		String outFileName=fileName.substring(0,fileName.length()-1)+".mp4";
		String dirName=serverProperties.getProperty("usersdir")+ownerName+"/"+fileName;
		String completeName=dirName+"stream.mpd";
		String outName=dirName+outFileName;
		System.out.println(completeName);
		System.out.println(outName);
		System.out.println(numStreams);
		VideoDash.videoMp4(completeName, outName,numStreams-1);
		response.sendRedirect("/ServidorMpegDashJorge/users/"+ownerName+"/"+fileName+outFileName);
	}
	@Override
	public void destroy() {
		modelo.close();
		
	}
	
}
