# Define a variable for classpath
CLASS_PATH = ../bin
JAVA_HOME = /usr/lib/jvm/java-8-openjdk-amd64

# Define a virtual path for .class in the bin directory
vpath %.class $(CLASS_PATH)/ffmpeg_jni/

# $* matches the target filename without the extension
# Pre JDK-10. JDK 10 removes the javah utility, need to use javac -h instead [TO CHECK]


all :libVideoDash.so VideoDash
libVideoDash.so : ffmpeg_jni_VideoThumbnail.c ffmpeg_jni_VideoThumbnail.h libVideoDash.c libVideoDash.h 
	gcc -shared -fpic -I"$(JAVA_HOME)/include" -I"$(JAVA_HOME)/include/linux" -L"/usr/local/lib" ffmpeg_jni_VideoThumbnail.c libVideoDash.c -lavcodec -lavformat -lavutil -lswscale -o $@
VideoDash 	 : ffmpeg_jni_VideoThumbnail.c ffmpeg_jni_VideoThumbnail.h libVideoDash.c ffmpeg_jni_VideoDash.h VideoDash.c VideoDash.h
	gcc -g -O0 -I"$(JAVA_HOME)/include" -I"$(JAVA_HOME)/include/linux" -L"/usr/local/lib" ffmpeg_jni_VideoThumbnail.c libVideoDash.c  VideoDash.c -lavcodec -lavformat -lavutil -lswscale -lm -o $@
# $@ matches the target, $< matches the first dependency
#libVideoThumbnail.so : ffmpeg_jni_VideoThumbnail.c ffmpeg_jni_VideoThumbnail.h
	#gcc -v -shared -fpic -I"$(JAVA_HOME)/include" -I"$(JAVA_HOME)/include/linux" -L"/usr/local/lib"  $<  -lavcodec -lavformat -lavutil -lswscale -o $@

#libVideoDashMux.so : ffmpeg_jni_VideoDashMux.c ffmpeg_jni_VideoDashMux.h
#	gcc -v -shared -fpic -I"$(JAVA_HOME)/include" -I"$(JAVA_HOME)/include/linux" -L"/usr/local/lib"  $<  -lavcodec -lavformat -lavutil -lswscale -o $@

clean :
	rm libVideoDash.so VideoDash
	
install:
#	cp libVideoThumbnail.so /usr/lib
	cp libVideoDash.so /usr/lib