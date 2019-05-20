/*
 * libçVideodash.c
 *
 *  Created on: 1 may. 2019
 *      Author: brad
 */

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
//#include <libswscale/swscale_internal.h>
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

typedef struct{
	int v_index;
	int a_index;
}streamIndex;

#define RESAMPLE_DROP -1
#define RESAMPLE_REPEAT 1
//#define RESAMPLE_NOTHING 0

typedef struct{
	int width;
	int height;
	int cfr;
	int max_rate;
	int buf_size;
	uint8_t extradata[4];
}vCodecPars;

typedef struct{
	int inRFPS;
	float inFPS;
	int ptspf;
	int dropF; //Cuantos frames sobran por segundo (borrar)
	int repeatF; //Cuantos faltan (repetir)
	int nDrop ; //Cada cuantos borrar
	int nRepeat; //Cada cuantos repetir
	//int nRepeatSec=-1; //Cada cuantoos repetir otra vez(30000/1001,24000/1001)
	int doRepeat;
	int frameInsideSeg;
	float lastpos;
	float position;
	int num_frame;
	int last_repeat;
	int repeated_frames;
	int timebase_den;
}ResampleContext;

const int outFramerate=25;
const int max_bframes=16;
int response=0;

static void logging(const char *fmt, ...);
int getVideoDash( const char* inputFileName,const char* outputDir);
AVFormatContext* init_in_fctx(const char* inputFileName);
streamIndex choose_input_streams(AVFormatContext* inFCtx);
AVCodecContext* init_in_a_cod_ctx(AVFormatContext* inFCtx,int streamIndex);
AVCodecContext* init_in_v_cod_ctx(AVFormatContext* inFCtx,int streamIndex);
AVFormatContext* init_out_fctx(const char *outputDir);
AVCodecContext* init_out_a_cod_ctx(AVFormatContext* outFCtx,AVCodecContext* inACCtx);
AVCodecContext* init_out_v_cod_ctx(AVFormatContext* outFCtx,vCodecPars params);
ResampleContext* init_resample_context(AVFormatContext* inFCtx,int StreamIndex);
int read_packet(AVFormatContext* inFCtx,AVPacket* inPacket);
int decode_v(AVCodecContext* inVCCtx,AVPacket* inPacket,AVFrame* inFrame);
int resample_drop_check(ResampleContext* resCtx,AVFrame* inFrame);
int resample_update_pts(ResampleContext* resCtx,AVFrame* inFrame);
int resample_fix_i_frames(ResampleContext* resCtx,AVFrame* inFrame);
int resample_check_repeat(ResampleContext* resCtx,AVFrame* inFrame);
int scale_frame(struct SwsContext* sclCtx,vCodecPars codecPars,AVFrame* outFrame,AVFrame* inFrame,int inVHeight);
int encode_frame_v(AVCodecContext* outVCCtx,AVFrame* outFrame, AVPacket* outPacket);
int write_packet(AVFormatContext* outFCtx,AVPacket* outPacket);
int resample_fix_repeated_frame(ResampleContext* resCtx,AVFrame* inFrame);


JNIEXPORT jint JNICALL Java_ffmpeg_1jni_VideoDash_getVideoDash
(JNIEnv *env, jobject obj, jstring jFilename,jstring jOutputdir){
	const char* fileName=(*env)->GetStringUTFChars(env,jFilename,NULL);
	const char* outputDir=(*env)->GetStringUTFChars(env,jOutputdir,NULL);
	return getVideoDash(fileName,outputDir);
}

int getVideoDash(const char* inputFileName,const char* outputDir){	//
	//ENTRADA
	streamIndex bestIndexes;
	AVFormatContext *inFCtx=NULL;	//DEMUXER
	AVCodecContext *inACCtx=NULL;	//VIDEO DECO
	AVCodecContext *inVCCtx=NULL;	///AUDIO DECO
	AVPacket *inPacket=NULL;
	AVFrame *inFrame=NULL;
	int inVWidth=0;
	int inVHeight=0;
	//SALIDA
	AVFormatContext *outFCtx=NULL;	//MUXER
	AVOutputFormat *outFmt=NULL;
	AVCodecContext *outACCtx=NULL;
	AVCodecContext *outVCCtxs[3];
	//AVStream *outAStream=NULL;
	/*AVStream *outVLowStream=NULL;
	AVStream *outVMidStream=NULL;
	AVStream *outVHighStream=NULL;*/
	AVPacket *outPacket=NULL;
	AVFrame *outFrame=NULL;
	//ESCALADO
	//struct SwsContext *sclHighCtx=NULL;
	struct SwsContext *sclCtxs[3];
	ResampleContext *resCtx=NULL;
	vCodecPars codecPars[3] = {
			{426,240,23,608000,1216000,{1,100,8,21}},
			{854,480,22,1216000,2432000,{1,100,8,30}},
			{1280,720,21,2496000,4992000,{1,100,8,31}}
	};




	int repeat=0;
	int numQualities=0;

	inFCtx=init_in_fctx(inputFileName);
	bestIndexes=choose_input_streams(inFCtx);
	inVWidth=inFCtx->streams[bestIndexes.v_index]->codecpar->width;
	inVHeight=inFCtx->streams[bestIndexes.v_index]->codecpar->height;

	if(inVHeight>=720)//Cuantas calidadeS????
		numQualities=3;
	else if(inVHeight>=480)
		numQualities=2;
	else
		numQualities=1;
	inVCCtx=init_in_v_cod_ctx(inFCtx,bestIndexes.v_index);
	inACCtx=init_in_a_cod_ctx(inFCtx,bestIndexes.a_index);

	resCtx=init_resample_context(inFCtx,bestIndexes.v_index);
	inFrame = av_frame_alloc();
	outFrame = av_frame_alloc();
	inPacket = av_packet_alloc();
	outPacket = av_packet_alloc();
	if (!(inFrame&&outFrame&&inPacket&&outPacket))	{
		logging("failed to allocated memory for AVFrames and AVPackets");return -1;
	}
	for(int i=0;i<numQualities;i++){
		codecPars[i].width=(inVCCtx->width*codecPars[i].height/inVCCtx->height/2*2);//MANTENER EL ANCHO DEL VIDEO ORIGINAL!!!!!!pero mantener ancho par
		sclCtxs[i]=sws_getContext(inVCCtx->width,inVCCtx->height,inVCCtx->pix_fmt,codecPars[i].width,codecPars[i].height,	AV_PIX_FMT_YUV420P,	 SWS_BILINEAR, NULL, NULL, NULL);
		if(!sclCtxs[i]){
			logging("Impossible to create scale context for the scaling ");return-1;
		}
	}

	outFCtx=init_out_fctx(outputDir);//+ muxer optons
	getVideoThumb(inputFileName); //PROBAR
	outACCtx=init_out_a_cod_ctx(outFCtx,inACCtx);

	for(int i=0;i<numQualities;i++){
			outVCCtxs[i]=init_out_v_cod_ctx(outFCtx,codecPars[i]);
		}
	//loop
	response = avformat_write_header(outFCtx,NULL);
	if (response < 0) {
		logging("Error while writing file header: %s", av_err2str(response));
		return response;
	}
	while(1){
		response=read_packet(inFCtx,inPacket);								//READ
		if(response==AVERROR_EOF){
			break;
		}//Salida por ahora
		if(inPacket->stream_index==bestIndexes.v_index){			//IF(IS_VIDEO))

			response=decode_v(inVCCtx,inPacket,inFrame);//GOT FRAME?
			if(response==AVERROR(EAGAIN)){
				continue;
			}
			if(response==AVERROR_EOF){
				break;
			}
			//logging("      inFrame: [%d] pts %d", resCtx->num_frame,inFrame->pts);
			response=resample_drop_check(resCtx,inFrame);//DROP FRAME???
			if(response==RESAMPLE_DROP){
				logging("Dropped");
				continue;
			}

			resample_update_pts(resCtx,inFrame);
			resample_fix_i_frames(resCtx,inFrame);
			do{
				repeat=0;
				//logging("outpts:  %d frameInsideSeg %d     %d",inFrame->pts,resCtx->frameInsideSeg,inFrame->pts/512);
				//logging("resampled Frame: [%d] pts %d, frameInsideSeg %d", resCtx->num_frame,inFrame->pts,resCtx->frameInsideSeg);
				for(int i=0;i<numQualities;i++){
					scale_frame(sclCtxs[i],codecPars[i],outFrame,inFrame,inVHeight);
					response=encode_frame_v(outVCCtxs[i],outFrame,outPacket);
					//av_frame_unref(outFrame);
					if(response==AVERROR(EAGAIN)){
						continue;
					}
					outPacket->stream_index=i+1;
					write_packet(outFCtx,outPacket);
					//frame_unref??
				}
				if(resample_check_repeat(resCtx,inFrame)){
					repeat=1;
					resample_fix_repeated_frame(resCtx,inFrame);
					//logging("repeat!");
				}
			}while(repeat);
			av_packet_unref(inPacket);//packte unref
		}else if(inPacket->stream_index==bestIndexes.a_index){ //ES AUDIO
			inPacket->stream_index=0;
			write_packet(outFCtx,inPacket);
		}

	}
	response = av_write_trailer(outFCtx);
	if (response < 0) {
		logging("Error while writing the trailer in the file: %s", av_err2str(response));
		return response;
	}
	av_dump_format(inFCtx, 0, inputFileName, 0);
	av_dump_format(outFCtx, 0, outputDir, 1);

	logging("induration: %d",(int)inFCtx->duration);
	logging("outduration: %d, (%ds)",(int)outFCtx->duration,(int)outFCtx->duration/12800);
	logging("instreamduration: %d",(int)inFCtx->streams[0]->duration);
	logging("outstreamduration: %d, (%ds)",(int)outFCtx->streams[0]->duration,(int)outFCtx->streams[0]->duration/12800);
	logging("nbframes: %d",(int)inFCtx->streams[0]->nb_frames);
	logging("outnbframes: %d",(int)outFCtx->streams[0]->nb_frames);




	//***************************************************LIMPIAR************************************************************************************
//(ARREGLAR)
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
	for(int i=0;i<numQualities;i++){
		avcodec_free_context(&outVCCtxs[i]);
		av_freep(&sclCtxs[i]);
	}


	avformat_free_context(outFCtx);

	return 0;
}

	//ENTRADA *********************************************************ENTRADA**************************************************************************

	AVFormatContext* init_in_fctx(const char* inputFileName){
		//Reservar memoria para FORMAT context
		AVFormatContext* inFCtx = avformat_alloc_context();
		if (!inFCtx) {
			logging("ERROR could not allocate memory for Format Context");
			return NULL;
		}
		//Cargar el archivo de entrada
		response=avformat_open_input(&inFCtx, inputFileName, NULL, NULL);
		if (response<0) {
			logging("ERROR could not open input file");
			return NULL;
		}
		return inFCtx;
	}

	streamIndex choose_input_streams(AVFormatContext* inFCtx){
		streamIndex bestIndexes={-1,-1};
		//Carga la información de los streams
		response=(avformat_find_stream_info(inFCtx,  NULL));
		if  (response < 0) {
			logging("ERROR could not get the stream info");
			return bestIndexes;
		}
		//Codec y parametros de los streams
		bestIndexes.v_index = -1;
		bestIndexes.a_index = -1;
		// para cada stream comprobar si es de video o audio, guardar el mejor de cada
		for (int i = 0; i < inFCtx->nb_streams; i++)
		{
			AVCodecParameters *codecPars =  NULL;
			codecPars = inFCtx->streams[i]->codecpar;
			if(codecPars->codec_type==AVMEDIA_TYPE_VIDEO){
				if(bestIndexes.v_index == -1||codecPars->bit_rate>inFCtx->streams[bestIndexes.v_index]->codecpar->bit_rate){
					bestIndexes.v_index=i;
				}
			}else if(codecPars->codec_type==AVMEDIA_TYPE_AUDIO){
				if(bestIndexes.a_index == -1||codecPars->bit_rate>inFCtx->streams[bestIndexes.a_index]->codecpar->bit_rate){
					bestIndexes.a_index=i;
				}
			}else{
				continue;
			}
		}
		if(bestIndexes.a_index==-1){
			logging("Advertencia! no se ha encontrado un apista de audio");
		}
		if(bestIndexes.v_index==-1){
			logging("Error! no se ha encontrado ninguna pista de video");
		}
		return bestIndexes;
	}

	//INIT AUDIO CODEC CONTEXT
	AVCodecContext* init_in_a_cod_ctx(AVFormatContext* inFCtx,int streamIndex){

		AVCodec* decoA = avcodec_find_decoder(inFCtx->streams[streamIndex]->codecpar->codec_id);
		if(decoA==NULL){
			logging("No se ha encontrado codec para decodificar el audio");
			return NULL;
		}
		AVCodecContext* inACCtx= avcodec_alloc_context3(decoA);
		if(!inACCtx){
			logging("No se ha podido reservar memoria para el decoder de audio");
			return NULL;
		}
		response = avcodec_parameters_to_context(inACCtx, inFCtx->streams[streamIndex]->codecpar);
		if(response<0){
			logging("No se ha podido copiar los parametros para el  decoder de audio: %s", av_err2str(response));
			return NULL;
		}
		response=avcodec_open2(inACCtx,decoA,NULL);
		if(response<0){
			logging("imposible abrir codec de entrada de audio: %s",av_err2str(response));
			return NULL;
		}
		return inACCtx;
	}

	AVCodecContext* init_in_v_cod_ctx(AVFormatContext* inFCtx,int streamIndex){
		AVCodec* decoV = avcodec_find_decoder(inFCtx->streams[streamIndex]->codecpar->codec_id);
		AVCodecContext* inVCCtx= avcodec_alloc_context3(decoV);
		if(!inVCCtx){
			logging("No se ha podido reservar memoria para el decoder de video");
			return NULL;
		}
		response = avcodec_parameters_to_context(inVCCtx, inFCtx->streams[streamIndex]->codecpar);
		if(response<0){
			logging("No se ha podido copiar los parametros para el  decoder de video: %s", av_err2str(response));
			return NULL;
		}
		response=avcodec_open2(inVCCtx,decoV,NULL);
		if(response<0){
			logging("imposible abrir codec de entrada de video: %s",av_err2str(response));
		}
		return inVCCtx;
	}
		/* MOVER
	inVBitrate=inVStream->codecpar->bit_rate;
	inVWidth=inVStream->codecpar->width;
	inVHeight=inVStream->codecpar->height;
	int inVFps=inVStream->avg_frame_rate.num;//USO???
	logging("Input fps: %d ",inVStream->avg_frame_rate.num);
	//inVStream->time_base=1/inVStream->avg_frame_rate;
*/

	//SALIDA*******************************************************************SALIDA*******************************************************************
	//************************************************************************************************************************************************

	AVFormatContext* init_out_fctx(const char *outputDir){//+ muxer optons
		AVFormatContext* outFCtx;
		AVOutputFormat* outFmt;
		char out_file[256]="";
		strcpy(out_file,outputDir);
		strcat(out_file,"/stream.mpd");
		struct stat st = {0};
		if (stat(outputDir, &st) == -1) { //Si no existe el directorio, crearlo
			mkdir(outputDir, 0700);
		} //SI YA EXISTE: ERROR???? TODO

		//Cargar el archivo de salida, como output del FormatContext;
		response=avformat_alloc_output_context2(&outFCtx, NULL, NULL, out_file);
		if (!outFCtx||response<0) {
			logging("Could not create output context\n");
			return NULL;
		}
		outFmt = av_guess_format("dash", NULL, NULL);
		outFCtx->oformat = outFmt;



		response=av_opt_set(outFCtx->priv_data,"adaptation_sets","id=0,streams=v id=1,streams=a",0); //Correspondencia de los streams a los adaptation sets (0 video, 1 audio)
		if(response<0)logging("error %s adaptation sets",av_err2str(response));
		response=av_opt_set(outFCtx->priv_data,"seg_duration","5",0);  //DURACION DE SEGMENTO
		if(response<0)logging("error %s seg duration",av_err2str(response));
		response=av_opt_set(outFCtx->priv_data,"use_template","1",0);
		if(response<0)logging("error %s use template",av_err2str(response));
		response=av_opt_set(outFCtx->priv_data,"use_timeline","1",0);
		if(response<0)logging("error %s use timeline",av_err2str(response));


		return outFCtx;
	}


	//OUT STREAMS & CODECS*****************************************CODEC_CTX*******************************************************************************


	//**************************************************************AUDIO****************************************************************************************

	AVCodecContext* init_out_a_cod_ctx(AVFormatContext* outFCtx,AVCodecContext* inACCtx){

		AVCodec *audioEnc=avcodec_find_encoder_by_name("aac");
		if(audioEnc==NULL){
			logging("codec not found for (aac)");
			return NULL;
		}
		AVStream* outAStream = avformat_new_stream(outFCtx, audioEnc);
		if (!outAStream) {
			logging("Failed allocating audio output stream.");
			return NULL;
		}
		AVCodecContext* outACCtx = avcodec_alloc_context3(audioEnc);
		outACCtx->codec_type = AVMEDIA_TYPE_AUDIO;
		outACCtx->sample_fmt = audioEnc->sample_fmts[0];
		outACCtx->sample_rate= inACCtx->sample_rate;

		outACCtx->compression_level=1;
		outACCtx->channels =inACCtx->channels>2?2:inACCtx->channels;
		if(outACCtx->channels>1){
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
		uint8_t extradataaudio[5]={18,16,86,229,0}; //17 is mp4a.40.2 profile, 144 is 2 channel  {17,144,86,66,229} funciona
		outACCtx->extradata_size=5;
		outACCtx->extradata=extradataaudio;
		outACCtx->codec_tag =  MKTAG('m', 'p', '4', 'a');
		avcodec_parameters_from_context(outAStream->codecpar,outACCtx);
		response=avcodec_open2(outACCtx,audioEnc,NULL);
		if(response<0){
			logging("imposible abrir codec de salida de audio (aac) : %s",av_err2str(response));
		}
		return outACCtx;
	}


	//***************************************VIDEO  ******************************************************************************************
	//*************************************INIT VIDEO CONTEXT (COMUN) **************************************************************************************
	AVCodecContext* init_out_v_cod_ctx(AVFormatContext* outFCtx,vCodecPars params){
		AVCodec* videoEnc=avcodec_find_encoder(AV_CODEC_ID_H264);   //AV_CODEC_ID_H264)
		if(videoEnc==NULL){
			logging("codec not found for (h264)");
			return NULL;
		}
		AVStream* outVStream = avformat_new_stream(outFCtx, videoEnc);
		if (!outVStream) {
			logging("Failed allocating video low output stream.");
			return NULL;
		}
		AVCodecContext* outVCCtx = avcodec_alloc_context3(videoEnc);
		if(outVCCtx==NULL){
			logging("Failed allocating video low output codec context");
			return NULL;
		}
		outVCCtx->width = params.width;
		outVCCtx->height = params.height;
		response=av_opt_set_int(outVCCtx->priv_data,"crf",params.cfr,0);
		if(response<0){	//BORRAR
			logging("Cannot set codec option: crf(23) invalid value");
			return NULL;
		}
		response=av_opt_set_int(outVCCtx, "bufsize",params.buf_size,0);
		if(response<0){
			logging("Cannot set codec option: bufsize(HIGH) invalid value");
			return NULL;
		}
		response=av_opt_set_int(outVCCtx, "maxrate",params.max_rate,0);
		if(response<0){
			logging("Cannot set codec option:maxrate(HIGH)");
			return NULL;
		}
		uint8_t *extradata=av_malloc(sizeof(uint8_t)*4);//Escribir extra_data para que el muxer escriba correctamente los archivos stream.mpd y init-stream0.mp4
		for(int i=0;i<4;i++){
			extradata[i]=params.extradata[i];
		}
		outVCCtx->extradata=extradata; //(100,64,21 = (avc1.644015 = avc1 profile high preset 2.1)
		outVCCtx->extradata_size=4;

		//CONSTANTES
		outVCCtx->codec_type = AVMEDIA_TYPE_VIDEO;
		outVCCtx->time_base = (AVRational){1, outFramerate};
		outVCCtx->framerate = (AVRational){outFramerate, 1};
		outVCCtx->gop_size = outFramerate*10;
		outVCCtx->keyint_min = outFramerate*5;
		outVCCtx->max_b_frames = max_bframes;
		outVCCtx->pix_fmt = AV_PIX_FMT_YUV420P;
		outVCCtx->profile=FF_PROFILE_H264_HIGH;
		av_opt_set(outVCCtx->priv_data,"preset","slow",0);


		response=avcodec_parameters_from_context(outVStream->codecpar,outVCCtx);
		if(response<0){
			logging("Cannot copy params fro context");
			return NULL;
		}
		outVStream->avg_frame_rate=(AVRational){25,1};
		outVStream->time_base=(AVRational){1,25};
		response=avcodec_open2(outVCCtx,videoEnc,NULL);
		if(response<0){
			logging("imposible abrir codec de salida de video calidad baja (h264) : %s",av_err2str(response));
		}
		return outVCCtx;
	}



	ResampleContext* init_resample_context(AVFormatContext* inFCtx,int StreamIndex){
		ResampleContext *resCtx=malloc(sizeof(ResampleContext));
		resCtx->doRepeat=0;
		resCtx->dropF=-1;
		resCtx->repeatF=-1;
		resCtx->frameInsideSeg=0;
		resCtx->nDrop=-1;
		resCtx->nRepeat=-1;
		resCtx->lastpos=0;
		resCtx->position=0;
		resCtx->num_frame=0;
		resCtx->last_repeat=0;
		resCtx->repeated_frames=0;

		resCtx->inFPS=((float)inFCtx->streams[StreamIndex]->avg_frame_rate.num/inFCtx->streams[StreamIndex]->avg_frame_rate.den);
		resCtx->inRFPS=(int)ceil(resCtx->inFPS);
		resCtx->ptspf = inFCtx->streams[StreamIndex]->avg_frame_rate.den*(inFCtx->streams[StreamIndex]->time_base.den/inFCtx->streams[StreamIndex]->avg_frame_rate.num);
		resCtx->timebase_den=inFCtx->streams[StreamIndex]->time_base.den;
		if(resCtx->inRFPS>25){
			resCtx->dropF=resCtx->inRFPS-25;
			resCtx->nDrop=resCtx->inRFPS/resCtx->dropF;
			}else if(resCtx->inRFPS<25){
				resCtx->repeatF=25-resCtx->inRFPS;
				resCtx->nRepeat=resCtx->inRFPS/resCtx->repeatF;
			}

		return resCtx;
	}

	//***********************************************************************************************************************************
	//***************************************************** L O O P **********************************************************************
	// **********************************************************************************************************************************

		//*********************************LEER PAQUETE DE LA ENTRADA*****************************************
	int read_packet(AVFormatContext* inFCtx,AVPacket* inPacket){
		response = av_read_frame(inFCtx, inPacket);
		if (response==AVERROR_EOF){
			logging("fin de la entrada");
			return response;
		} else if(response<0){
			logging("AV error cannot read frame :%s ",av_err2str(response));
			return response;
		}
		return 0;
	}
	int decode_v(AVCodecContext* inVCCtx,AVPacket* inPacket,AVFrame* inFrame){
		int response = avcodec_send_packet(inVCCtx, inPacket);
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
		response = avcodec_receive_frame(inVCCtx, inFrame);
		if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
			logging("Need more packets %s", av_err2str(response));
			return response;
		} else if (response < 0) {
			logging("Error while receiving a frame from the decoder: %s", av_err2str(response));
			return response;
		}
		return 0;
	}

			//////////////////////////////////////////MANIPULAR FRAMES ( TIEMPO,....)/////////////////////////////////////////////////
				//DROP CHECK
	int resample_drop_check(ResampleContext* resCtx,AVFrame* inFrame){
		resCtx->num_frame=(inFrame->pts/resCtx->ptspf);//%(inStream->avg_frame_rate.num/inStream->avg_frame_rate.den);
		if(resCtx->nDrop>0){
			if(resCtx->num_frame%resCtx->nDrop==resCtx->nDrop-1){ //el ultimo
				return RESAMPLE_DROP;
			}
		}
		return 0;
	}

	int resample_update_pts(ResampleContext* resCtx,AVFrame* inFrame){


		resCtx->lastpos=resCtx->position;
		resCtx->position=(float)inFrame->pts/resCtx->timebase_den;
		//logging("inpts: %d position: %f frameInsideSeg %d (%d)   %d",inFrame->pts,resCtx->position,resCtx->frameInsideSeg,resCtx->timebase_den,inFrame->pts*resCtx->inRFPS/resCtx->timebase_den);
		int seg=(int)resCtx->position;
		if(floor(resCtx->position)-floor(resCtx->lastpos)>0){
			resCtx->frameInsideSeg=0;
			resCtx->repeated_frames=0;
		}
		int64_t outpts=seg*12800+resCtx->frameInsideSeg*512;
		inFrame->pts = outpts;
		float outposition=(float)inFrame->pts/12800;
		//resCtx->position=
		resCtx->frameInsideSeg++;
		//if(inFrame->key_frame)
		//	logging("KeyFrame: %d (%d) (resampled)", inFrame->pts/512,inFrame->pts);
		//%(inStream->avg_frame_rate.num/inStream->avg_frame_rate.den);

		return 0;
	}
				//FIX I_FRAMES
	int resample_fix_i_frames(ResampleContext* resCtx,AVFrame* inFrame){//ON NECESITA RESAMPLE CONTEXT PARA NADA (POR AHORA)
		resCtx->num_frame=(inFrame->pts/512);
		if(((resCtx->num_frame))%125==0){//Keyframes fijos cada 74 ??frames
			inFrame->pict_type=AV_PICTURE_TYPE_I;
			inFrame->key_frame=1;
			logging("KeyFrame: %d (%d)", resCtx->num_frame,inFrame->pts);
		}else{
			inFrame->pict_type=AV_PICTURE_TYPE_NONE;//el encoder elige
			inFrame->key_frame=0;
		}
		return 0;
	}

	int resample_check_repeat(ResampleContext* resCtx,AVFrame* inFrame){
		if(resCtx->nRepeat>0&&resCtx->last_repeat==0&&resCtx->repeated_frames<resCtx->repeatF){
			if(resCtx->frameInsideSeg%resCtx->nRepeat==0){ //el ultimo
				resCtx->repeated_frames++;
				resCtx->doRepeat=1;
				resCtx->last_repeat=1;
				return RESAMPLE_REPEAT;
			}
		}
		resCtx->last_repeat=0;
		return 0;
	}

	//************************************ESCALA EL FRAME (SCALE) *****************************************************
	int scale_frame(struct SwsContext* sclCtx,vCodecPars codecPars,AVFrame* outFrame,AVFrame* inFrame,int inVHeight){
		uint8_t *src_data[4], *dst_data[4];
		int src_linesize[4],dst_linesize[4];
		int dst_bufsize;
		int srcHeight = inVHeight;
		int destHeight = codecPars.height;
		outFrame->width=codecPars.width;
		outFrame->height=destHeight;
		outFrame->format=AV_PIX_FMT_YUV420P;
		outFrame->coded_picture_number=inFrame->coded_picture_number;
		outFrame->pict_type=inFrame->pict_type;
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
		response=sws_scale(sclCtx,(const uint8_t * const*)src_data,src_linesize,0,srcHeight,dst_data,dst_linesize);
		if(response<0){
			logging("imposible escalar el frame ");
			return response;
		}
		outFrame->pts=inFrame->pts;
		return 0;
	}

	int encode_frame_v(AVCodecContext* outVCCtx,AVFrame* outFrame, AVPacket* outPacket){
					//****************************************ENVIA FRAME ( ENNCODE ) ******************************************************
		response = avcodec_send_frame(outVCCtx, outFrame);
		av_frame_unref(outFrame);
		if (response == AVERROR(EAGAIN) || response == AVERROR_EOF ) {
			logging("Error necesitas mas input: %s", av_err2str(response)); //CREO QUE ESO ON PUEDE PASAR
			return response;
		}else if(response==AVERROR(EINVAL)){
			logging("Error del codec????: %s", av_err2str(response)); //codec not opened, refcounted_frames not set, it is a decoder, or requires flush
			return response;
		}else if(response < 0){
			logging("Error while sending a frame to the encoder: %s", av_err2str(response));
			return response;
		}
		//******************************************RECIBE PAQUETE (ENCODEC)**************************************************************
		//ESTO QUIZAS ESTA MAL Y HAY QUE AÑADIR OTRO WHILE AHORA LO VEREMOS
		response = avcodec_receive_packet(outVCCtx, outPacket);
		if (response == AVERROR(EAGAIN)) {
			return response;
		}else if (response == AVERROR_EOF){
			logging("FIN!!!!!!: %s", av_err2str(response));
			return response;
		}
		else if (response < 0) {
			logging("Error while receiving a packet from the enncoder: %s", av_err2str(response));
			return response;
		}
		return 0;
	}

	int write_packet(AVFormatContext* outFCtx,AVPacket* outPacket){
		//********************************************ESCRIBIR PAQUETE (MUX) ********************************************************************
		//response=av_interleaved_write_frame(outFCtx, outPacket);
		response=av_write_frame(outFCtx, outPacket);	//No he notado cambios a 15fps
		if (response==AVERROR(EAGAIN)) {
			logging( "Error muxing packet: more data needed\n");
			return response;
		}else if (response==AVERROR_EOF) {
			logging( "End\n");
			return response;
		}else if(response < 0){
			logging("Error muxing packet: %s",av_err2str(response));
			return response;
		}
		return 1;
	}

	int resample_fix_repeated_frame(ResampleContext* resCtx,AVFrame* inFrame){
					//inFrame->pts = inFrame->pts+resCtx->ptspf;
					//resCtx->position=(float)inFrame->pts/resCtx->timebase_den;
					//resCtx->num_frame=(inFrame->pts/512)%outFramerate;
					resCtx->frameInsideSeg++;
					inFrame->repeat_pict=1;
					inFrame->pts+=512;
					resCtx->num_frame++;
					return 0;
				}

		/*
	//av_packet_unref(inPacket); y este donde va¿?
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


*/

	//**************************************************ESCRIBIR FINAL DEL ARCHIVO ***************************************************************

static void logging(const char *fmt, ...)
{
	va_list args;
	fprintf( stderr, "LOG: " );
	va_start( args, fmt );
	vfprintf( stderr, fmt, args );
	va_end( args );
	fprintf( stderr, "\n" );
}



