/*
 * VideoDash.c
 *
 *  Created on: 14 abr. 2019
 *      Author: brad
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "libVideoDash.h"
#include "ffmpeg_jni_VideoThumbnail.h"
#include "VideoDash.h"

void printusage();

int main(int argc,char* argv[]){
	char filename[256];
	char outdir[256];
	if(argc<3 || argc>=4){
		printusage();
		return -1;
	}
	strcpy(filename,argv[1]);
	strcpy(outdir,argv[2]);
	return getVideoDash(filename,outdir);

}

void printusage(){
	printf("Uso: VideoDash nombrearchivo dierctoriosalida\n");
}
