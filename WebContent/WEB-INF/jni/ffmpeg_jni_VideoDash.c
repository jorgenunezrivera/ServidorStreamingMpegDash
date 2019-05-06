/*
 * ffmpeg_jni_VideoDash.c
 *
 *  Created on: 8 abr. 2019
 *      Author: Jorge Nuñez Rivera
 */
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>

#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <inttypes.h>
#include "ffmpeg_jni_VideoThumbnail.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

static void logging(const char *fmt, ...);
int getVideoDash( const char* inputFileName);

JNIEXPORT jint JNICALL Java_ffmpeg_1jni_VideoDash_getVideoDash
(JNIEnv *env, jobject obj, jstring jFilename){
	const char* fileName=(*env)->GetStringUTFChars(env,jFilename,NULL);
	return getVideoDash(fileName);
}

int getVideoDash(const char* inputFileName){
	//ENTRADA
	AVFormatContext *inFCtx=NULL;	//DEMUXER
	AVCodecContext *inACCtx=NULL;	//VIDEO DECO
	AVCodecContext *inVCCtx=NULL;	///AUDIO DECO
	AVStream *inAStream=NULL;
	AVStream *inVStream=NULL;
	AVPacket *inPacket=NULL;
	AVFrame *inFrame=NULL;
	int inVBitrate=0;
	int inVWidth=0;
	int inVHeight=0;
	int inAChannels=0;
	//SALIDA
	AVFormatContext *outFCtx=NULL;	//MUXER
	AVOutputFormat *outFmt=NULL;
	AVCodecContext *outACCtx=NULL;
	AVCodecContext *outVLowCCtx=NULL;
	AVCodecContext *outVMidCCtx=NULL;
	AVCodecContext *outVHighCCtx=NULL;
	AVStream *outAStream=NULL;
	AVStream *outVLowStream=NULL;
	AVStream *outVMidStream=NULL;
	AVStream *outVHighStream=NULL;
	AVPacket *outPacket=NULL;
	AVFrame *outFrame=NULL;
	//ESCALADO
	struct SwsContext *sclMidCtx=NULL;
	struct SwsContext *sclLowCtx=NULL;

	int response=0;
	int frame_index=0;
	int outAChannels=2;//2 por defecto, si la entrada es mono 1

	//ENTRADA *********************************************************ENTRADA**************************************************************************

	//Reservar memoria para FORMAT context
	inFCtx = avformat_alloc_context();
	if (!inFCtx) {
		logging("ERROR could not allocate memory for Format Context");
		return -1;
	}
	//Cargar el archivo de entrada
	response=avformat_open_input(&inFCtx, inputFileName, NULL, NULL);
	if (response<0) {
		logging("ERROR could not open input file");
		return response;
	}
	//Carga la información de los streams
	response=(avformat_find_stream_info(inFCtx,  NULL));
	if  (response < 0) {
		logging("ERROR could not get the stream info");
		return response;
	}
	//Codec y parametros de los streams
	int best_video_stream_index = -1;
	int best_audio_stream_index = -1;
	// para cada stream comprobar si es de video o audio, guardar el mejor de cada
	for (int i = 0; i < inFCtx->nb_streams; i++)
	{
		AVCodecParameters *codecPars =  NULL;
		codecPars = inFCtx->streams[i]->codecpar;
		if(codecPars->codec_type==AVMEDIA_TYPE_VIDEO){
			if(best_video_stream_index == -1||codecPars->bit_rate>inFCtx->streams[best_video_stream_index]->codecpar->bit_rate){
				best_video_stream_index=i;
			}
		}else if(codecPars->codec_type==AVMEDIA_TYPE_AUDIO){
			if(best_audio_stream_index == -1||codecPars->bit_rate>inFCtx->streams[best_audio_stream_index]->codecpar->bit_rate){
				best_audio_stream_index=i;
			}
		}else{
			continue;
		}
	}
	if(best_audio_stream_index==-1){
		logging("Advertencia! no se ha encontrado un apista de audio");
	}
	if(best_video_stream_index==-1){
		logging("Error! no se ha encontrado ninguna pista de video");
		return -1;
	}
	inAStream = inFCtx->streams[best_audio_stream_index];
	AVCodec* decoA = avcodec_find_decoder(inAStream->codecpar->codec_id);
	if(decoA==NULL){
		logging("No se ha encontrado codec para decodificar el audio");
		return -1;
	}
	inACCtx= avcodec_alloc_context3(decoA);
	if(!inACCtx){
		logging("No se ha podido reservar memoria para el decoder de audio");
		return -1;
	}
	response = avcodec_parameters_to_context(inACCtx, inAStream->codecpar);
	if(response<0){
		logging("No se ha podido copiar los parametros para el  decoder de audio: %s", av_err2str(response));
		return -1;
	}
	response=avcodec_open2(inACCtx,decoA,NULL);
	if(response<0){
		logging("imposible abrir codec de entrada de audio: %s",av_err2str(response));
	}
	inAChannels=inACCtx->channels;
	if(inAChannels==1){
		outAChannels=1;
	}else if(inAChannels<1){
		logging("No hay canales de audio!!!!");//POR RESOLVER
		return -1;
	}

	inVStream = inFCtx->streams[best_video_stream_index];
	AVCodec* decoV = avcodec_find_decoder(inVStream->codecpar->codec_id);
	inVCCtx= avcodec_alloc_context3(decoV);
	if(!inVCCtx){
		logging("No se ha podido reservar memoria para el decoder de video");
		return -1;
	}
	response = avcodec_parameters_to_context(inVCCtx, inVStream->codecpar);
	if(response<0){
		logging("No se ha podido copiar los parametros para el  decoder de video: %s", av_err2str(response));
		return -1;
	}
	inVBitrate=inVStream->codecpar->bit_rate;
	inVWidth=inVStream->codecpar->width;
	inVHeight=inVStream->codecpar->height;
	int inVFps=inVStream->avg_frame_rate.num;
	logging("Input fps: %d ",inVStream->avg_frame_rate.num);
	//inVStream->time_base=1/inVStream->avg_frame_rate;


	//SALIDA*******************************************************************SALIDA*******************************************************************
	//************************************************************************************************************************************************
	//Nombre del archivo de salida
	char out_dir[256]="";
	strcpy(out_dir,inputFileName);
	char* lastdot = strrchr(out_dir,'.');
	lastdot[0]='\0';
	strcat(out_dir,"-DASH");
	char out_file[256]="";
	strcpy(out_file,out_dir);
	strcat(out_file,"/stream.mpd");
	struct stat st = {0};
	if (stat(out_dir, &st) == -1) { //Si no existe el directorio, crearlo
	    mkdir(out_dir, 0700);
	} //SI YA EXISTE: ERROR????
	getVideoThumb(inputFileName);

	//Cargar el archivo de salida, como output del FormatContext;
	response=avformat_alloc_output_context2(&outFCtx, NULL, NULL, out_file);
	if (!outFCtx||response<0) {
		logging("Could not create output context\n");
		return response;
	}
	outFmt = av_guess_format("dash", NULL, NULL);
	outFCtx->oformat = outFmt;
	//outFCtx->max_chunk_duration=5000000;


	//OUT STREAMS & CODECS*****************************************STREAMS*******************************************************************************


	//**************************************************************AUDIO****************************************************************************************

	AVCodec *audioEnc=avcodec_find_encoder_by_name("aac");
	if(audioEnc==NULL){
		logging("codec not found for (aac)");
		return -1;
	}
	outAStream = avformat_new_stream(outFCtx, audioEnc);
	if (!outAStream) {
		logging("Failed allocating audio output stream.");
		return AVERROR_UNKNOWN;
	}
	outACCtx = avcodec_alloc_context3(audioEnc);
	outACCtx->codec_type = AVMEDIA_TYPE_AUDIO;
	outACCtx->sample_fmt = audioEnc->sample_fmts[0];
	outACCtx->sample_rate= inACCtx->sample_rate;

	outACCtx->compression_level=1;
	outACCtx->channels =outAChannels;
	if(outAChannels>1){
		outACCtx->channel_layout=AV_CH_LAYOUT_STEREO;
	}else{
		outACCtx->channel_layout=AV_CH_LAYOUT_MONO;
	}
	outACCtx->bit_rate = inACCtx->bit_rate;
	outACCtx->time_base = (AVRational){1, 25};
	outACCtx->profile=FF_PROFILE_AAC_LOW;
	outACCtx->frame_size =inACCtx->frame_size;
	if (outFCtx->oformat->flags & AVFMT_GLOBALHEADER)
		outACCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	uint8_t extradataaudio[5]={17,144,86,229,1}; //17 is mp4a.40.2 profile, 144 is 2 channel  {17,144,86,66,229} funciona
	outACCtx->extradata_size=5;
	outACCtx->extradata=extradataaudio;
	outACCtx->codec_tag =  MKTAG('m', 'p', '4', 'a');
	avcodec_parameters_from_context(outAStream->codecpar,outACCtx);


	//***************************************VIDEO  ******************************************************************************************
	//*************************************CODEC (COMUN) **************************************************************************************
	AVCodec* videoEnc=avcodec_find_encoder(AV_CODEC_ID_H264);   //AV_CODEC_ID_H264)
	if(videoEnc==NULL){
		logging("codec not found for (h264)");
		return -1;
	}

	//************************************ PERFIL BAJO **************************************************************************************
	outVLowStream = avformat_new_stream(outFCtx, videoEnc);
	if (!outVLowStream) {
		logging("Failed allocating video low output stream.");
		return AVERROR_UNKNOWN;
	}
	outVLowCCtx = avcodec_alloc_context3(videoEnc);
	if(outVLowCCtx==NULL){
		logging("Failed allocating video low output codec context");
		return -1;
	}
	//ffmpeg  -i /home/brad/Descargas/MortalKombatTrailer.mp4 -map 0:v -map 0:a -c:a aac -c:v libx264 -crf 23  -profile:v:0 high -preset:v:0 slow -bf 16
	//-keyint_min 120 -g 240 -sc_threshold 40 -b_strategy 1  -force_key_frames "expr:eq(mod(n,120),0)" -vsync cfr -ac 2 -r 24 -maxrate:v:0 2000k  -bufsize 4000k
	outVLowCCtx->codec_type = AVMEDIA_TYPE_VIDEO;
	outVLowCCtx->bit_rate = inVBitrate/3; //?? CALCULAR SEGUN ENTRADA??
	outVLowCCtx->width = (inVWidth/6)*2;	//MULTIPLOS DE 2??????         FIJAR O CALCULAR?
	outVLowCCtx->height = (inVHeight/6)*2; //????????????????
	outVLowCCtx->time_base = (AVRational){1, 25};

	outVLowCCtx->framerate = (AVRational){25, 1};
	outVLowCCtx->gop_size = 250;
	outVLowCCtx->keyint_min = 125;
	outVLowCCtx->max_b_frames = 16;
	outVLowCCtx->pix_fmt = AV_PIX_FMT_YUV420P;
	outVLowCCtx->profile=FF_PROFILE_H264_HIGH;
	response=av_opt_set(outVLowCCtx->priv_data,"preset","slow",0);
	if(response<0){
		logging("Cannot set codec option");
		return -1;
	}
	response=av_opt_set(outVLowCCtx->priv_data,"crf","23",0);
	if(response<0){
		logging("Cannot set codec option");
		return -1;
	}
	char bufsize[9];
	sprintf(bufsize,"%d",inVBitrate*2); //FIJAR
	response=av_opt_set(outVLowCCtx, "bufsize",bufsize,AV_OPT_SEARCH_CHILDREN);
	if(response<0){
		logging("Cannot set codec option: bufsize(HIGH) invalid value");
		return -1;
	}
	char maxrate[9];
	sprintf(maxrate,"%d",inVBitrate);//FIJAR TODO
	response=av_opt_set(outVLowCCtx, "maxrate",maxrate,0);
	if(response<0){
		logging("Cannot set codec option:maxrate(HIGH)");
		return -1;
	}
	uint8_t *extradatalow=av_malloc(sizeof(uint8_t)*4);//Escribir extra_data para que el muxer escriba correctamente los archivos stream.mpd y init-stream0.mp4
	extradatalow[0]=1;extradatalow[1]=100;extradatalow[2]=8;extradatalow[3]=21;  //liberado por free_codec_context()
	outVLowCCtx->extradata=extradatalow; //(100,64,21 = (avc1.644015 = avc1 profile high preset 2.1)
	outVLowCCtx->extradata_size=4;

	response=avcodec_parameters_from_context(outVLowStream->codecpar,outVLowCCtx);
	if(response<0){
		logging("Cannot copy params fro context");
		return -1;
	}
	outVLowStream->avg_frame_rate=(AVRational){25,1};
	outVLowStream->time_base=(AVRational){1,25};



	//*********************************** PERFIL MEDIO **************************************************************************************************
	outVMidStream = avformat_new_stream(outFCtx, videoEnc);
	if (!outVMidStream) {
		logging("Failed allocating video mid output stream.");
		return AVERROR_UNKNOWN;
	}
	outVMidCCtx = avcodec_alloc_context3(videoEnc);
	if(outVMidCCtx<0){
		logging("Failed allocating video mid output codec context");
		return -1;
	}
	outVMidCCtx->codec_type = AVMEDIA_TYPE_VIDEO;
	outVMidCCtx->bit_rate = inVBitrate; //?? CALCULAR SEGUN ENTRADA??
	outVMidCCtx->width = (inVWidth/3)*2;	//MULTIPLOS DE 2??????
	outVMidCCtx->height = (inVHeight/3)*2; //????????????????
	outVMidCCtx->time_base = (AVRational){1, 25};
	outVMidCCtx->framerate = (AVRational){25, 1};
	outVMidCCtx->gop_size = 250;
	outVMidCCtx->keyint_min = 125;
	outVMidCCtx->max_b_frames = 16;
	outVMidCCtx->pix_fmt = AV_PIX_FMT_YUV420P;
	outVMidCCtx->extradata_size=4;
	outVMidCCtx->profile=FF_PROFILE_H264_HIGH;
	uint8_t *extradatamid=av_malloc(sizeof(uint8_t)*4);//Escribir extra_data para que el muxer escriba correctamente los archivos stream.mpd y init-stream0.mp4
	extradatamid[0]=1;extradatamid[1]=100;extradatamid[2]=8;extradatamid[3]=30;
	outVMidCCtx->extradata=extradatamid; //(100,08,20 = (avc1.64401E = avc1 profile high preset 3.0)
	outVMidCCtx->extradata_size=4;
	response=av_opt_set(outVMidCCtx->priv_data,"preset","slow",0);
		if(response<0){
			logging("Cannot set codec option");
			return -1;
		}
		response=av_opt_set(outVMidCCtx->priv_data,"crf","23",0);
		if(response<0){
			logging("Cannot set codec option");
			return -1;
		}
		char midbufsize[9];
		sprintf(midbufsize,"%d",inVBitrate*2); //FIJAR
		response=av_opt_set(outVMidCCtx, "bufsize",midbufsize,AV_OPT_SEARCH_CHILDREN);
		if(response<0){
			logging("Cannot set codec option: bufsize(HIGH) invalid value");
			return -1;
		}
		char midmaxrate[9];
		sprintf(midmaxrate,"%d",inVBitrate);//FIJAR TODO
		response=av_opt_set(outVMidCCtx, "maxrate",midmaxrate,0);
		if(response<0){
			logging("Cannot set codec option:maxrate(HIGH)");
			return -1;
		}
	avcodec_parameters_from_context(outVMidStream->codecpar,outVMidCCtx);
	outVMidStream->avg_frame_rate=(AVRational){25,1};
	outVMidStream->time_base=(AVRational){1,25};

	//((******************************PERFIL ALTO ****************************************************************************************))
	outVHighStream = avformat_new_stream(outFCtx, videoEnc);
	if (!outVHighStream) {
		logging("Failed allocating video high output stream.");
		return AVERROR_UNKNOWN;
	}
	outVHighCCtx = avcodec_alloc_context3(videoEnc);
	if(outVHighCCtx<0){
		logging("Failed allocating video High output codec context");
		return -1;
	}
	outVHighCCtx->codec_type = AVMEDIA_TYPE_VIDEO;
	outVHighCCtx->bit_rate = inVBitrate*2; //?? CALCULAR SEGUN ENTRADA??
	outVHighCCtx->width = inVWidth;	//MULTIPLOS DE 2??????
	outVHighCCtx->height = inVHeight; //????????????????
	outVHighCCtx->time_base = (AVRational){1, 25};
	outVHighCCtx->framerate = (AVRational){25, 1};
	outVMidCCtx->gop_size = 250;
	outVHighCCtx->keyint_min = 125;
	outVHighCCtx->max_b_frames = 16;
	outVHighCCtx->pix_fmt = AV_PIX_FMT_YUV420P;
	outVHighCCtx->profile=FF_PROFILE_H264_HIGH;
	uint8_t *extradatahigh=av_malloc(sizeof(uint8_t)*4);//Escribir extra_data para que el muxer escriba correctamente los archivos stream.mpd y init-stream0.mp4
	extradatahigh[0]=1;extradatahigh[1]=100;extradatahigh[2]=8;extradatahigh[3]=31;
	outVHighCCtx->extradata_size=4;
	outVHighCCtx->extradata=extradatahigh;
	response=av_opt_set(outVHighCCtx->priv_data,"preset","slow",0);
	if(response<0){
		logging("Cannot set codec option");
		return -1;
	}

	char highbufsize[9];
	sprintf(highbufsize,"%d",inVBitrate*2);

	response=av_opt_set(outVHighCCtx, "bufsize",highbufsize,AV_OPT_SEARCH_CHILDREN);
	if(response==AVERROR_OPTION_NOT_FOUND){
		logging("Cannot set codec option: bufsize(HIGH): option not found");
		return -1;
	}else if(response==AVERROR(ERANGE)){
		logging("Cannot set codec option: bufsize(HIGH): value out of range");
		return -1;
	}else if(response==AVERROR(EINVAL)){
		logging("Cannot set codec option: bufsize(HIGH) invalid value");
		return -1;
	}
	char highmaxrate[9];
	sprintf(highmaxrate,"%d",inVBitrate);
	response=av_opt_set(outVHighCCtx, "maxrate",highmaxrate,0);
	if(response<0){
		logging("Cannot set codec option:maxrate(HIGH)");
		return -1;
	}
	response=av_opt_set(outVHighCCtx->priv_data,"crf","21",0);
	if(response<0){
		logging("Cannot set codec option");
		return -1;
	}
	avcodec_parameters_from_context(outVHighStream->codecpar,outVHighCCtx);
	outVHighStream->avg_frame_rate=(AVRational){25,1};
	outVHighStream->time_base=(AVRational){1,25};

	//ABRIR CODECS*******************************************ABRIR COEDCS***************************************************
	response=avcodec_open2(outACCtx,audioEnc,NULL);
	if(response<0){
		logging("imposible abrir codec de salida de audio (aac) : %s",av_err2str(response));
	}
	response=avcodec_open2(outVLowCCtx,videoEnc,NULL);
	if(response<0){
		logging("imposible abrir codec de salida de video calidad baja (h264) : %s",av_err2str(response));
	}
	response=avcodec_open2(outVMidCCtx,videoEnc,NULL);
	if(response<0){
		logging("imposible abrir codec de salida de video calidad media (h264) : %s",av_err2str(response));
	}
	response=avcodec_open2(outVHighCCtx,videoEnc,NULL);
	if(response<0){
		logging("imposible abrir codec de salida de video calidad alta (h264) : %s",av_err2str(response));
	}
	response=avcodec_open2(inVCCtx,decoV,NULL);
	if(response<0){
		logging("imposible abrir codec de entrada de video: %s",av_err2str(response));
	}

	//**************************************OPCIONES DEL MUXER *************************************************************************


	response=av_opt_set(outFCtx->priv_data,"adaptation_sets","id=0,streams=v id=1,streams=a",0); //Correspondencia de los streams a los adaptation sets (0 video, 1 audio)
	if(response<0)logging("error %s adaptation sets",av_err2str(response));
	response=av_opt_set(outFCtx->priv_data,"seg_duration","1",0);  //DURACION DE SEGMENTO
	if(response<0)logging("error %s seg duration",av_err2str(response));
	response=av_opt_set(outFCtx->priv_data,"use_template","1",0);
	if(response<0)logging("error %s use template",av_err2str(response));
	response=av_opt_set(outFCtx->priv_data,"use_timeline","1",0);
	if(response<0)logging("error %s use timeline",av_err2str(response));
	//response=av_opt_set(outFCtx->priv_data,"format_options","frag_duration=5000000",0);
	//if(response<0)logging("error %s format options->frag duration",av_err2str(response));
	logging("time base: %d ",outFCtx->streams[0]->time_base);

	//*************************************************************ESCRIBIR HEADER *******************************************************************************
	response = avformat_write_header(outFCtx,NULL);
	if (response < 0) {
		logging("Error while writing file header: %s", av_err2str(response));
		return response;
	}
	//Reservando memoria para frame y  packet
	inFrame = av_frame_alloc();
	if (!inFrame)
	{
		logging("failed to allocated memory for AVFrame");
		return -1;
	}
	outFrame = av_frame_alloc();
	if (!outFrame)
	{
		logging("failed to allocated memory for AVFrame");
		return -1;
	}
	//Reservando memoria para packet

	inPacket = av_packet_alloc();
	if (!inPacket)
	{
		logging("failed to allocated memory for AVPacket");
		return -1;
	}


	outPacket = av_packet_alloc();
	if (!outPacket)
	{
		logging("failed to allocated memory for AVPacket");
		return -1;
	}
	// Contexto del Escalador ( de escala no de escalar el everest)////////////////////////////////////////////////////////////////////////////////////////
	sclMidCtx =sws_getContext(inVWidth,	inVHeight,inVCCtx->pix_fmt,(inVWidth/3)*2,(inVHeight/3)*2,	AV_PIX_FMT_YUV420P,	 SWS_BILINEAR, NULL, NULL, NULL);
	if (!sclMidCtx) {
		logging("Impossible to create scale context for the mid scaling ");
	}
	sclLowCtx =sws_getContext(inVWidth,	inVHeight,inVCCtx->pix_fmt,(inVWidth/6)*2,(inVHeight/6)*2,	AV_PIX_FMT_YUV420P,	 SWS_BILINEAR, NULL, NULL, NULL);
	if (!sclMidCtx) {
		logging("Impossible to create scale context for the low scaling ");
	}

	////////////////////////////////Configuracion de los fps (drops o repeats)///////////////////////////////////////////
	int inRFPS = (int)ceil((float)inVStream->avg_frame_rate.num/inVStream->avg_frame_rate.den);
	int ptspf = inVStream->avg_frame_rate.den*(inVStream->time_base.den/inVStream->avg_frame_rate.num);
	int dropF =0; //Cuantos frames sobran por segundo (borrar)
	int repeatF=0; //Cuantos faltan (repetir)
	int nDrop =-1; //Cada cuantos borrar
	int nRepeat=-1; //Cada cuantos repetir
	int nRepeatSec=-1; //Cada cuantoos repetir otra vez(30000/1001,24000/1001)
	int doRepeat=0;
	if(inRFPS>25){
		dropF=inRFPS-25;
		nDrop=inRFPS/dropF;
	}else if(inRFPS<25){
		repeatF=25-inRFPS;
		nRepeat=inRFPS/repeatF;
	}
	int frameInsideSeg=0;
	float lastpos;
	float position;
	int num_frame;
	int last_repeat;
	int repeated_frames=0;
	/*if(inVStream->avg_frame_rate.den==1001){
				nRepeatSec=
	}*/
	logging("INFPS: %d repeat = %d nrepeat=%d",inRFPS,repeatF,nRepeat);
	logging("In tmebase: %d/%d",inVStream->time_base.num,inVStream->time_base.den);
	logging("In framerate: %d/%d",inVStream->r_frame_rate.num,inVStream->r_frame_rate.den);
	logging("In framerate: %d/%d",inVStream->avg_frame_rate.num,inVStream->avg_frame_rate.den);

	//***********************************************************************************************************************************
	//***************************************************** L O O P **********************************************************************
	// **********************************************************************************************************************************
	while (1)
	{
		AVCodecContext *inCC, *outCC,*aOutCC[3];
		AVStream *inStream,*outStream,*aOutStream[3];
		int isvideo=0;

		//*********************************LEER PAQUETE DE LA ENTRADA*****************************************
		response = av_read_frame(inFCtx, inPacket);
		if (response==AVERROR_EOF){
			logging("fin de la entrada");
			break;
		} else if(response<0){
			logging("AV error cannot read frame :%s ",av_err2str(response));
			return response;
		}  else {

			//SI ES AUDIO ASIGNAR EL CODEC DE ENTRADA Y DE SALIDA Y LOS STREAMS DE ENTRADA Y SALIDA
			if(inFCtx->streams[inPacket->stream_index]->codecpar->codec_type==AVMEDIA_TYPE_AUDIO){
				inCC=inACCtx;
				outCC=outACCtx;
				isvideo=0;
				inStream=inAStream;
				outStream=outAStream;
				//SI ES VIDEO ASIGNAR ENTRADAS Y ARRAYS DE SALIDAS (CODECS Y STREAMS)
			}else if(inFCtx->streams[inPacket->stream_index]->codecpar->codec_type==AVMEDIA_TYPE_VIDEO){
				inCC=inVCCtx;
				aOutCC[0]=outVLowCCtx;
				aOutCC[1]=outVMidCCtx;
				aOutCC[2]=outVHighCCtx;
				isvideo=1;
				inStream=inVStream;

			}else{
				continue;
			}

			//*************************************ENVIA PAQUETE (DECODE)  *******************************************
			int response = avcodec_send_packet(inCC, inPacket);
			if (response == AVERROR(EINVAL)) {
				logging("Error while sending a packet to the decoder: invalid argument: %s", av_err2str(response));
				return response;
			}else if (response == AVERROR(EAGAIN)){
				logging("Error while sending a packet to the decoder: waiting to recive %s", av_err2str(response));
			}else if (response<0){
				logging("Error while sending a packet to the decoder: %s", av_err2str(response));
				return response;
			}
			//************************************RECIBE FRAME (DECODE) ***********************************************
			response = avcodec_receive_frame(inCC, inFrame);
			if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
				logging("Need more packets %s", av_err2str(response));
				continue;
			} else if (response < 0) {
				logging("Error while receiving a frame from the decoder: %s", av_err2str(response));
				return response;
			}

			//////////////////////////////////////////MANIPULAR FRAMES ( TIEMPO,....)/////////////////////////////////////////////////

			if(isvideo){
				num_frame=(inFrame->pts/ptspf);//%(inStream->avg_frame_rate.num/inStream->avg_frame_rate.den);
				if(nDrop>0){
					if(num_frame%nDrop==nDrop-1){ //el ultimo
						//logging("frame: inFrame : %d (%fs) [%d] :...DROP!!...",inFrame->pts,num_frame);
						continue;

						//FRAME DROP???
					}
				}
				lastpos=position;
				position=(float)inFrame->pts/inStream->time_base.den;

				//logging("frame: inFrame : %d (%fs)",inFrame->pts/inVStream->avg_frame_rate.den,position);
				int seg=(int)position;

				if(floor(position)-floor(lastpos)>0){
					frameInsideSeg=0;
					repeated_frames=0;
				}
				int64_t outpts=seg*aOutStream[0]->time_base.den+frameInsideSeg*512;
				inFrame->pts = outpts;
				//logging("seg: %d position : %f lastpos: %f",seg,position,lastpos,frameInsideSeg);


				position=(float)inFrame->pts/aOutStream[0]->time_base.den;
				num_frame=(inFrame->pts/512);//%(inStream->avg_frame_rate.num/inStream->avg_frame_rate.den);

				//num_frame=(inFrame->pts/512);
				frameInsideSeg++;
				//logging("frame: outFrame : %d (%fs) [%d]",inFrame->pts/512,position,frameInsideSeg);

				if(((inFrame->pts/512))%125==0){//Keyframes fijos cada 74 ??frames
					inFrame->pict_type=AV_PICTURE_TYPE_I;
					inFrame->key_frame=1;
					//logging("Keyframe: frame : %d (%d,%d)",inFrame->coded_picture_number,inFrame->pts/512,inFrame->pts/512);
					//inFrame->pict_type=AV_PICTURE_TYPE_NONE;//el encoder elige
				}else{
					inFrame->pict_type=AV_PICTURE_TYPE_NONE;//el encoder elige
					//logging("videoframe: outframe :%d",inFrame->pts/512);
				}
			}else{
				//logging("timebase: in %d/%d out %d/%d",inAStream->time_base.num,inAStream->time_base.den,outAStream->time_base.num,outAStream->time_base.den);
				//inFrame->pts/=2;
				logging("audio frame: outFrame : %d ",inFrame->pts/1024);

			}
			do{//CUIDADDO REPITE FRAME DE AUDIO TAMBIEN
				last_repeat=doRepeat;
				doRepeat=0;
				if(nRepeat>0&&last_repeat==0&&repeated_frames<repeatF&&isvideo){
					//logging("frame : %d,repeated frames = %d,repeatF %d",frameInsideSeg,repeated_frames,repeatF);
					if(frameInsideSeg%nRepeat==0){ //el ultimo
						repeated_frames++;
						doRepeat=1;
						//logging("Manda repetir");
					}
				}
				//*********************************MULTIPLICANDO FRAME**********************************************************
				int times;
				//Si es video hace 3 pasadas del bucle for
				//si es audio hace solo una
				if(isvideo){
					times=3;
				}else {
					times=1;
				}
				for(int i=0;i<times;i++){ //1 vez para audio o 3 para video
					//************************************ESCALA EL FRAME (SCALE) *****************************************************
					uint8_t *src_data[4], *dst_data[4];
					int src_linesize[4],dst_linesize[4];
					int dst_bufsize;
					//outFrame=av_frame_alloc();
					if(outFrame==NULL){
						logging("No se ha podido reservar espacio para el frame de salida");
						return -1;
					}
					if(isvideo){
						outStream=aOutStream[i];
						if(i==0){//Low
							int width=(inVWidth/6)*2;
							int height = (inVHeight/6)*2;
							outFrame->width=width;
							outFrame->height=height;
							outFrame->format=AV_PIX_FMT_YUV420P;
							outFrame->coded_picture_number=inFrame->coded_picture_number;
							response=av_frame_get_buffer(outFrame,0);
							if(response<0){
								logging("imposible reservar buferes para el frame de salida, %s",av_err2str(response));
								return response;
							}
							for(int i=0;i<4;i++){
								dst_data[i]=outFrame->data[i];
								dst_linesize[i]=outFrame->linesize[i];
								src_data[i]=inFrame->data[i];
								src_linesize[i]=inFrame->linesize[i];
							}
							//response=av_frame_ref(outFrame,inFrame);

							response=sws_scale(sclLowCtx,(const uint8_t * const*)src_data,src_linesize,0,inVHeight,dst_data,dst_linesize);
							if(response<0){
								logging("imposible escalar el frame ");
								return response;
							}
							outFrame->pts=inFrame->pts;


						}else if(i==1){//MID
							int width=(inVWidth/3)*2;
							int height = (inVHeight/3)*2;
							dst_bufsize = response;
							outFrame->width=width;
							outFrame->height=height;
							outFrame->format=AV_PIX_FMT_YUV420P;
							outFrame->coded_picture_number=inFrame->coded_picture_number;
							response=av_frame_get_buffer(outFrame,0);
							if(response<0){
								logging("imposible reservar buferes para el frame de salida, %s",av_err2str(response));
								return response;
							}
							for(int i=0;i<4;i++){
								dst_data[i]=outFrame->data[i];
								dst_linesize[i]=outFrame->linesize[i];
								src_data[i]=inFrame->data[i];
								src_linesize[i]=inFrame->linesize[i];
							}
							response=sws_scale(sclMidCtx,(const uint8_t * const*)src_data,src_linesize,0,inVHeight,dst_data,dst_linesize);
							if(response<0){
								logging("imposible escalar el frame ");
								return response;
							}
							outFrame->pts=inFrame->pts;

						}else{
							response=av_frame_ref(outFrame,inFrame);
							if(response<0){
								logging("imposible copiar frame e entrada a salida");
								return response;
							}

						}


					}else{	//AUDIO
						response=av_frame_ref(outFrame,inFrame);
						if(response<0){
							logging("imposible copiar frame e entrada a salida");
							return response;
						}
						//ESTARA AQUI EL FALLO??
						//logging("inFrame: nsamples:%d,format:%d,channel layout:%d, pts:%d,linesize %d",inFrame->nb_samples,inFrame->format,inFrame->channel_layout,inFrame->pts/1024,inFrame->linesize[0]);
						//outFrame->nb_samples= outACCtx->frame_size;
						//outFrame->format= outACCtx->sample_fmt;
						//outFrame->channel_layout=outACCtx->channel_layout;
						//logging("outFrame: nsamples:%d,format:%d,channel layout:%d, pts:%d, linesize %d",outFrame->nb_samples,outFrame->format,outFrame->channel_layout,outFrame->pts/1024,outFrame->linesize[0]);

					}


					//outFrame->coded_picture_number = av_rescale_q_rnd(outFrame->coded_picture_number, inStream->time_base, outStream->time_base, (AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));

					//****************************************ENVIA FRAME ( ENNCODE ) ******************************************************
					response = avcodec_send_frame(isvideo?aOutCC[i]:outCC, outFrame);
					if (response == AVERROR(EAGAIN) || response == AVERROR_EOF ) {
						logging("Error necesitas mas input: %s", av_err2str(response)); //CREO QUE ESO ON PUEDE PASAR
					}else if(response==AVERROR(EINVAL)){
						logging("Error del codec????: %s", av_err2str(response)); //codec not opened, refcounted_frames not set, it is a decoder, or requires flush
						return response;
					}else if(response < 0){
						logging("Error while sending a frame to the encoder: %s", av_err2str(response));
						return response;
					}
					av_frame_unref(outFrame);

					while(response>=0){
						//******************************************RECIBE PAQUETE (ENCODEC)**************************************************************
						response = avcodec_receive_packet(isvideo?aOutCC[i]:outCC, outPacket);
						if (response == AVERROR(EAGAIN)) {
							break;
						}else if (response == AVERROR_EOF){
							logging("FIN!!!!!!: %s", av_err2str(response));
							break;
						}
						else if (response < 0) {
							logging("Error while receiving a packet from the enncoder: %s", av_err2str(response));
							return response;
						}
						if(isvideo){
							outPacket->stream_index=i+1;
						}else{
							outPacket->stream_index=0;
						}

						if(isvideo)
						{}//logging("video packet : %d (%d)",outPacket->pts,outPacket->dts);
						else
							logging("audio packet : %d (%d) dts: (%d)",outPacket->pts/1024,outPacket->pts,outPacket->dts/1024);
						//********************************************ESCRIBIR PAQUETE (MUX) ********************************************************************
						//response=av_interleaved_write_frame(outFCtx, outPacket);
						response=av_write_frame(outFCtx, outPacket);	//No he notado cambios a 15fps
						if (response==AVERROR(EAGAIN)) {
							logging( "Error muxing packet: more data needed\n");
							break;
						}else if (response==AVERROR_EOF) {
							logging( "End\n");
							break;
						}else if(response < 0){
							logging("Error muxing packet: %s",av_err2str(response));
							break;
						}
					}
				}
				if(!isvideo){
					doRepeat=0;
				}
				//logging("dorepeat: %d",doRepeat);
				if(doRepeat){
					inFrame->pts = inFrame->pts+ptspf;
					position=(float)inFrame->pts/outStream->time_base.den;
					num_frame=(inFrame->pts/512)%outStream->avg_frame_rate.num;
					frameInsideSeg++;
					//logging("REPEAT! repeated: %d",repeated_frames);
					//logging("frame: inFrame : %d (%fs)",inFrame->pts*inVStream->avg_frame_rate.num/inVStream->time_base.den,position);
					//logging("frame: outFrame : %d (%fs) [%d]",inFrame->pts/512,position,frameInsideSeg);
				}
			}while(doRepeat); //el ultimo
		}
		av_packet_unref(inPacket);
	}
	//FLUSH CODECS
	//do{
	int  response2=0;

	logging("flushing decoder");
	response=avcodec_send_packet(inACCtx,NULL);
	logging("response: %d",response);
	do{
		response=avcodec_receive_frame(inACCtx,inFrame);
		logging("response: %d",response);
		logging("response: %d",avcodec_send_frame(outACCtx,inFrame));
		response2=avcodec_receive_packet(outACCtx,outPacket);
		logging("response2: %d",response2);
		logging("audio packet : %d (%d) dts: (%d)",outPacket->pts/1024,outPacket->pts,outPacket->dts/1024);
		av_interleaved_write_frame(outFCtx, outPacket);
	}while(response!=AVERROR_EOF);
	logging("flushing encoder");
	response=avcodec_send_frame(outACCtx,NULL);
	logging("response: %d",response);
	do{
		response=avcodec_receive_packet(outACCtx,outPacket);
		if(response<0)
			break;
		logging("response: %d",response);
		logging("audio packet : %d (%d) dts: (%d)",outPacket->pts/1024,outPacket->pts,outPacket->dts/1024);
		av_interleaved_write_frame(outFCtx, outPacket);
	}while(1);



	av_dump_format(inFCtx, 0, inputFileName, 0);
	av_dump_format(outFCtx, 0, out_file, 1);

	//**************************************************ESCRIBIR FINAL DEL ARCHIVO ***************************************************************
	response = av_write_trailer(outFCtx);
	if (response < 0) {
		logging("Error while writing the trailer in the file: %s", av_err2str(response));
		return response;
	}
	//***************************************************LIMPIAR************************************************************************************

	av_packet_unref(outPacket);
	av_packet_unref(inPacket);
	av_frame_unref(inFrame);
	av_frame_unref(outFrame);
	av_packet_free(&inPacket);
	av_packet_free(&outPacket);
	av_frame_free(&inFrame);
	av_frame_free(&outFrame);


	avcodec_free_context(&inACCtx);
	avcodec_free_context(&inVCCtx);
	avformat_close_input(&inFCtx);
	avformat_free_context(inFCtx);

	avcodec_free_context(&outACCtx);
	avcodec_free_context(&outVLowCCtx);
	avcodec_free_context(&outVHighCCtx);
	avcodec_free_context(&outVMidCCtx);


	avformat_free_context(outFCtx);

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
