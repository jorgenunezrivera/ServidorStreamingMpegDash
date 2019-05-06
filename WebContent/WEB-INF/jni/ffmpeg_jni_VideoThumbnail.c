/*
 * ffmpeg_jni_VideoThumbnail.cpp
 *
 *  Created on: 4 abr. 2019
 *      Author: brad
 */

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>

#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ffmpeg_jni_VideoThumbnail.h"
#include <stdarg.h>
#include <inttypes.h>





// print out the steps and errors
static void logging(const char *fmt, ...);
int getVideoThumb(const char* inputFileName);
// decode packets into frames

JNIEXPORT jint JNICALL Java_ffmpeg_1jni_VideoThumbnail_getVideoThumb
(JNIEnv* env, jobject thisObj, jstring jFilename){
	const char* inputFileName=(*env)->GetStringUTFChars(env,jFilename,NULL);
	return getVideoThumb(inputFileName);
}

int getVideoThumb(const char* inputFileName){
	AVOutputFormat* fmt=NULL;
	AVFormatContext *pInFormatContext=NULL;
	AVFormatContext *pOutFormatContext=NULL;
	AVCodec *pInCodec = NULL;
	AVCodecParameters *pInCodecParameters =  NULL;
	AVCodecContext *pInCodecContext = NULL;
	AVCodec *pOutCodec = NULL;
	AVCodecParameters *pOutCodecParameters =  NULL;
	AVCodecContext *pOutCodecContext = NULL;
	AVPacket* newPacket=NULL;
	AVStream* video_st=NULL;
	int response=0;
	const int in_w=480,in_h=272;

	//Nombre del archivo de salida,
	char out_file[256];
	strcpy(out_file,inputFileName);
	char* lastdot = strrchr(out_file,'.');
	lastdot[0]='\0';
	strcat(out_file,"-DASH/pre.jpg");

	//Reservar memoria para codec context
	pInFormatContext = avformat_alloc_context();
	if (!pInFormatContext) {
		logging("ERROR could not allocate memory for Format Context");
		return -1;
	}
	fmt = av_guess_format("mjpeg", NULL, NULL);
	//
	pOutFormatContext=avformat_alloc_context();
	pOutFormatContext->oformat = fmt;

	//Cargar el archivo de entrada
	response=avformat_open_input(&pInFormatContext, inputFileName, NULL, NULL);//hay que cerrar
	if (response<0) {
		logging("ERROR could not open input file %s",inputFileName);
		return response;
	}
	//Cargar el archivo de salida, como output del FormatContext;
	response=avio_open(&pOutFormatContext->pb,out_file, AVIO_FLAG_READ_WRITE);
	if(response<0){
		logging("Couldn't open output file. : %s",out_file);
		return response;
	}

	//Carga la información de los streams
	response=(avformat_find_stream_info(pInFormatContext,  NULL));
	if  (response < 0) {
		logging("ERROR could not get the stream info");
		return response;
	}
	//Codec y parametros de los streams
	int video_stream_index = -1;
	// para cada stream comprobar si es de video y si lo es guardar sus caracteristicas (Y salir porqu econ uno me llega y me da igual cual)
	for (int i = 0; i < pInFormatContext->nb_streams; i++)
	{
		AVCodecParameters *pLocalCodecParameters =  NULL;
		AVCodec *pLocalCodec = NULL;
		pLocalCodecParameters = pInFormatContext->streams[i]->codecpar;
		pLocalCodec = avcodec_find_decoder(pLocalCodecParameters->codec_id);
		if (pLocalCodec==NULL) {
			logging("ERROR unsupported codec!");
			return -1;
		}
		// para el ultimo stream de video guardamos codec y codecparameters
		if (pLocalCodecParameters->codec_type == AVMEDIA_TYPE_VIDEO) {
			video_stream_index = i;
			pInCodec = pLocalCodec;
			pInCodecParameters = pLocalCodecParameters;
		}
	}
	//Cargar codec para el formato de salida ?
	pOutCodec = avcodec_find_encoder(fmt->video_codec);
	if (pOutCodec==NULL) {
		logging("ERROR unsupported out video codec!");
		return -1;
	}

	//Reservando memoria para el CodecContext (ENTRADA)
	pInCodecContext = avcodec_alloc_context3(pInCodec);
	if (!pInCodecContext)
	{
		logging("failed to allocated memory for AVCodecContext");
		return -1;
	}
	//CArgandolo con los parametros obtenidos del stream
	if (avcodec_parameters_to_context(pInCodecContext, pInCodecParameters) < 0)
	{
		logging("failed to copy codec params to codec context");
		return -1;
	}
	//Inicializar codecContext
	if (avcodec_open2(pInCodecContext, pInCodec, NULL) < 0)
	{
		logging("failed to open codec through avcodec_open2");
		return -1;
	}
	//Reservando memoria para el CodecContext (SALIDA)

	pOutCodecContext = avcodec_alloc_context3(pOutCodec);
	if (!pOutCodecContext) {
		logging("failed to allocated memory for AVCodecContext (out)");
		return -1;
	} // Ya viene del stream??

	video_st=avformat_new_stream(pOutFormatContext,0);
	if(video_st==NULL){
		logging("failed to create output video stream");
		return -1;
	}
	pOutCodecParameters=video_st->codecpar;
	if (avcodec_parameters_to_context(pOutCodecContext, pOutCodecParameters) < 0)
	{
		logging("failed to copy codec params to codec context");
		return -1;
	}
	//pOutCodecContext->codec_id=fmt->video_codec;
	pOutCodecContext->codec_type = AVMEDIA_TYPE_VIDEO;
	pOutCodecContext->width = pInCodecContext->width;
	pOutCodecContext->height = pInCodecContext->height;
	pOutCodecContext->pix_fmt = AV_PIX_FMT_YUVJ420P; //TODO:Comprobar
	pOutCodecContext->time_base.num = 1;
	pOutCodecContext->time_base.den = 25;

	//Inicializar codecContext
	response = avcodec_open2(pOutCodecContext, pOutCodec, NULL);
	if(response<0)
	{
		logging("failed to open output codec through avcodec_open2");
		return response;
	}

	//Reservando memoria para frame y  packet
	AVFrame *pFrame = av_frame_alloc();
	if (!pFrame)
	{
		logging("failed to allocated memory for AVFrame");
		return -1;
	}
	AVPacket *pPacket = av_packet_alloc();
	if (!pPacket)
	{
		logging("failed to allocated memory for AVPacket");
		return -1;
	}

	// fill the Packet with data from the Stream
	while(response==0||response == AVERROR(EAGAIN))  {
		av_read_frame(pInFormatContext, pPacket) ;
		// if it's the video stream
		if (pPacket->stream_index == video_stream_index) {
			//Envia el paquete al decoder
			response = avcodec_send_packet(pInCodecContext, pPacket);
			if (response < 0) {
				logging("Error while sending a packet to the decoder: %s", av_err2str(response));
				return response;
			}
			response = avcodec_receive_frame(pInCodecContext, pFrame);
			if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
				logging("Error while receiving a frame from the decoder:time to send¿¿¿??? %s", av_err2str(response));
			} else if (response < 0) {
				logging("Error while receiving a frame from the decoder: %s", av_err2str(response));
				return response;
			}else{
				break;
			}
		}
	}
	/*
	pFrame->format=pOutCodecContext->pix_fmt;
	pFrame->width=pOutCodecContext->width
	pFrame->height=pOutCodecContext->height;
	*/
	response = avcodec_send_frame(pOutCodecContext, pFrame);

	if (response == AVERROR(EAGAIN) || response == AVERROR_EOF ) {
		logging("Error necesitas mas input: %s", av_err2str(response));
	}else if(response==AVERROR(EINVAL)){
		logging("Error del codec????: %s", av_err2str(response)); //codec not opened, refcounted_frames not set, it is a decoder, or requires flush
		return response;
	}else if(response < 0){
		logging("Error while sending a frame to the encoder: %s", av_err2str(response));
		return response;
	}
	newPacket= av_packet_alloc();
	if (!pPacket)
	{
		logging("failed to allocated memory for AVPacket");
		return -1;
	}
	response = avcodec_receive_packet(pOutCodecContext, newPacket);
	if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
		logging("NECESITO MAS FRAMES!!!!!!: %s", av_err2str(response));
		return response;
	} else if (response < 0) {
		logging("Error while receiving a packet from the enncoder: %s", av_err2str(response));
		return response;
	}
	response = avformat_write_header(pOutFormatContext,NULL);
	if (response < 0) {
		logging("Error while writing file header: %s", av_err2str(response));
		return response;
	}
	response = av_write_frame(pOutFormatContext, newPacket);
	if (response < 0) {
		logging("Error while writing packet in the file: %s", av_err2str(response));
		return response;
	}
	response = av_write_trailer(pOutFormatContext);
	if (response < 0) {
		logging("Error while writing the trailer in the file: %s", av_err2str(response));
		return response;
	}
	av_packet_unref(newPacket);
	av_packet_free(&newPacket);
	logging("Éxito");
	avcodec_close(pInCodecContext);
	avcodec_close(pOutCodecContext);
	av_frame_unref(pFrame);//Libera el frame
	av_packet_unref(pPacket);
	avformat_close_input(&pInFormatContext);
	avformat_free_context(pInFormatContext);
	avio_close(pOutFormatContext->pb);
	avformat_free_context(pOutFormatContext);
	av_packet_free(&pPacket);
	av_frame_free(&pFrame);
	avcodec_free_context(&pInCodecContext);
	avcodec_free_context(&pOutCodecContext);
	return 0;
}



static void logging(const char *fmt, ...)
{
	va_list args;
	fprintf( stderr, "LOG: " );
	va_start( args, fmt );
	vfprintf( stderr, fmt, args );
	va_end( args );
	fprintf( stderr, "\n" );
}


