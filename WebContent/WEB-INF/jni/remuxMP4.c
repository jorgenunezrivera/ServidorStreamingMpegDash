//REMUX DASH VIDEO INTO MP4 FOR DOWNLOAD

#include <libavformat/avformat.h>
#include "remuxMP4.h"

int get_num_streams(char* filename);
int remux_mp4(char* infilename,char* outfilename ,int stream_index);
char* get_video_info(char* filename);
AVFormatContext* init_out_fctx_mp4(const char *outputFileName);
AVFormatContext* init_in_fctx_dash(const char* inputFileName);

JNIEXPORT jint JNICALL Java_ffmpeg_1jni_VideoDash_getVideoMp4
  (JNIEnv *env, jobject obj, jstring jFilename,jstring jOutputdir,jint jStreamIndex){
	printf("JNI CALL\n");
	char* fileName=(*env)->GetStringUTFChars(env,jFilename,NULL);
	char* outputDir=(*env)->GetStringUTFChars(env,jOutputdir,NULL);
	int streamIndex=jStreamIndex;
	printf("C CALL\n");
	return remux_mp4(fileName,outputDir,streamIndex);
}

JNIEXPORT jstring JNICALL Java_ffmpeg_1jni_VideoDash_getVideoInfo
  (JNIEnv *env, jobject obj, jstring jFilename){
	char* fileName=(*env)->GetStringUTFChars(env,jFilename,NULL);
	return (*env)->NewStringUTF(env,get_video_info(fileName));
}

char* get_video_info(char* filename){
	printf("opening video info \n");
	char* info = malloc(sizeof(char)*256);
	strcpy(info,"");
	AVFormatContext* FC=init_in_fctx_dash(filename);
	if(!FC)
		return "error";
	avformat_find_stream_info(FC, NULL);
	char nstreams[20]="";
	char duration[20]="";
	char bitrate[20]="";
	char width[20] ="";
	char height[20] ="";

	sprintf(duration,"%" PRId64 ";",((FC->duration+5000)/AV_TIME_BASE));
	//sprintf(duration,"%d;",(FC->streams[0]->nb_frames/25));
	strcat(info,duration);
	//sprintf(duration,"%d;",(FC->streams[0]->nb_frames/25));
	//strcat(info,duration);

	sprintf(nstreams,"%d;",FC->nb_streams-1);
	strcat(info,nstreams);
	for(int i=0;i<FC->nb_streams-1;i++){
		sprintf(bitrate,"%ld,",FC->streams[i]->codecpar->bit_rate);
		strcat(info,bitrate);
		sprintf(width,"%d,",FC->streams[i]->codecpar->width);
		strcat(info,width);
		sprintf(height,"%d;",FC->streams[i]->codecpar->height);
		strcat(info,height);
	}
	return info;
}



int get_num_streams(char* filename){
	AVFormatContext* FC=init_in_fctx_dash(filename);
	if(!FC)
		return -1;
	return FC->nb_streams;
}

int get_video_duration(char* filename){
	AVFormatContext* FC=init_in_fctx_dash(filename);
		if(!FC)
			return -1;
		return FC->streams[0]->duration/12800;
}

int get_stream_bitrate(char* filename,int numStream){
	AVFormatContext* FC=init_in_fctx_dash(filename);
		if(!FC)
			return -1;
		return FC->streams[numStream]->codecpar->bit_rate;
}

int get_stream_width(char* filename,int numStream){
	AVFormatContext* FC=init_in_fctx_dash(filename);
		if(!FC)
			return -1;
		return FC->streams[numStream]->codecpar->width;
}

int get_stream_height(char* filename,int numStream){
	AVFormatContext* FC=init_in_fctx_dash(filename);
		if(!FC)
			return -1;
		return FC->streams[numStream]->codecpar->width;
}

JNIEXPORT jint JNICALL Java_ffmpeg_1jni_VideoDash_getNumStreams  (JNIEnv * env, jobject obj, jstring jFilename){
	char* fileName=(*env)->GetStringUTFChars(env,jFilename,NULL);
	return get_num_streams(fileName);
}

JNIEXPORT jint JNICALL Java_ffmpeg_1jni_VideoDash_getVideoDuration  (JNIEnv * env, jobject obj, jstring jFilename){
	char* fileName=(*env)->GetStringUTFChars(env,jFilename,NULL);
	return get_video_duration(fileName);
}

JNIEXPORT jint JNICALL Java_ffmpeg_1jni_VideoDash_getStreamBitrate  (JNIEnv * env, jobject obj, jstring jFilename,jint numStream){
	char* fileName=(*env)->GetStringUTFChars(env,jFilename,NULL);
	int stream=numStream;
	return get_stream_bitrate(fileName,stream);
}

JNIEXPORT jint JNICALL Java_ffmpeg_1jni_VideoDash_getStreamWidth  (JNIEnv * env, jobject obj, jstring jFilename,jint numStream){
	char* fileName=(*env)->GetStringUTFChars(env,jFilename,NULL);
		int stream=numStream;
		return get_stream_width(fileName,stream);
}

JNIEXPORT jint JNICALL Java_ffmpeg_1jni_VideoDash_getStreamHeight  (JNIEnv * env, jobject obj, jstring jFilename,jint numStream){
	char* fileName=(*env)->GetStringUTFChars(env,jFilename,NULL);
			int stream=numStream;
			return get_stream_height(fileName,stream);
}

//GET VIDEO INFO
//PUEDE DEVOLVER UN STRUCT O TEXTO

int remux_mp4(char* infilename,char* outfilename,int stream_index){
	printf("Init contexts\n");
	AVFormatContext* inFC=init_in_fctx_dash(infilename);
	AVFormatContext* outFC=init_out_fctx_mp4(outfilename);
	AVCodec* audioCodec = avcodec_find_decoder(inFC->streams[inFC->nb_streams-1]->codecpar->codec_id);
	AVCodec* videoCodec = avcodec_find_decoder(inFC->streams[stream_index]->codecpar->codec_id);
	AVStream* outAStream = avformat_new_stream(outFC,audioCodec);
	if (!outAStream) {
		return -1;
	}
	AVStream* outVStream = avformat_new_stream(outFC,videoCodec);
	if (!outVStream) {
		return -1;
	}
	outVStream->codecpar=inFC->streams[stream_index]->codecpar;
	outVStream->time_base=(AVRational){1, 25};
	outAStream->codecpar=inFC->streams[inFC->nb_streams-1]->codecpar;
		AVPacket* packet=av_packet_alloc();
	av_dump_format(inFC, 0, infilename, 0);
	av_dump_format(outFC, 0, outfilename, 1);
	int response=avio_open(&outFC->pb,outfilename,AVIO_FLAG_WRITE);
	if (response < 0) {
		printf("Cannot open output file: %d\n",response);
		return response;
	}
	response = avformat_write_header(outFC,NULL);
	if (response < 0) {
		printf("%d\n",response);
		return response;
	}
	printf("header written\n");
	while(1){
		printf("Read frame\n");
		response=av_read_frame(inFC,packet);								//READ
			if(response==AVERROR_EOF){
				break;
			}//Salida por ahora
			if(packet->stream_index==stream_index){			//Video
				packet->stream_index=1;
				av_write_frame(outFC,packet);
			}
			if(packet->stream_index==inFC->nb_streams-1){//audio
				packet->stream_index=0;
				av_write_frame(outFC,packet);
			}
		}

		response = av_write_trailer(outFC);
		if (response < 0) {
			printf("error writng trailer\n");
			return response;
		}
		av_dump_format(inFC, 0, infilename, 0);
		av_dump_format(outFC, 0, outfilename, 1);
		printf("cleaning packet\n");
		av_packet_unref(packet);
		av_packet_free(&packet);
		printf("cleaning input\n");
		avformat_close_input(&inFC);
		avformat_free_context(inFC);
		printf("cleaning output\n");
		avio_closep(&outFC->pb);
		printf("cleaning output ctx\n");
		//avformat_free_context(outFC);

}

AVFormatContext* init_in_fctx_dash(const char* inputFileName){
		//Reservar memoria para FORMAT context
		AVFormatContext* inFCtx = avformat_alloc_context();
		if (!inFCtx) {
			return NULL;
		}
		//Cargar el archivo de entrada
		int response=avformat_open_input(&inFCtx, inputFileName, NULL, NULL);
		if (response<0) {
			return NULL;
		}
		return inFCtx;
	}

AVFormatContext* init_out_fctx_mp4(const char *outputFileName){//+ muxer optons
		AVFormatContext* outFCtx;
		AVOutputFormat* outFmt;
		//Cargar el archivo de salida, como output del FormatContext;
		int response=avformat_alloc_output_context2(&outFCtx, NULL, NULL, outputFileName);
		if (!outFCtx||response<0) {
			return NULL;
		}
		outFmt = av_guess_format("mp4", NULL, NULL);
		outFCtx->oformat = outFmt;

}
