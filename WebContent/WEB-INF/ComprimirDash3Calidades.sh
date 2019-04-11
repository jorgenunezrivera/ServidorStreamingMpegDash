if [ "$#" -ne 1 ]; then
    echo Uso: ComprimirDash3Calidades.sh nombre_fichero
    exit 1
else
    PATH=/home/brad/Bento4/bin:$PATH    
    FILENAME=$1
    FFMPEG=ffmpeg
    fname=${FILENAME%.*}
    fpath=${FILENAME%/*}
    echo $fname
    fextension=${FILENAME##*.}
    oextension=mp4
    echo $fextension
    echo $fname.$fextension
#Comprobar directorio

    if [ -d "$fname"-DASH ] ; then
        echo el directorio ya existe
        exit 2
    fi

    $FFMPEG -y -i $fname.$fextension -c:a aac -ac 2 -ab 128k -c:v libx264 -x264opts 'keyint=24:min-keyint=24:no-scenecut' -b:v 1500k -maxrate 1500k -bufsize 1000k -vf "scale=-2:720" "$fname"1500.$oextension

    $FFMPEG -y -i $fname.$fextension -c:a aac -ac 2 -ab 128k -c:v libx264 -x264opts 'keyint=24:min-keyint=24:no-scenecut' -b:v 800k -maxrate 800k -bufsize 500k -vf "scale=-2:540" "$fname"800.$oextension

    $FFMPEG -y -i $fname.$fextension -c:a aac -ac 2 -ab 128k -c:v libx264 -x264opts 'keyint=24:min-keyint=24:no-scenecut' -b:v 400k -maxrate 400k -bufsize 400k -vf "scale=-2:360" "$fname"400.$oextension

    $FFMPEG -i $fname.$fextension -frames:v 1 -f image2 "$fname"pre.jpg



#mp4fragment "$fname"1500.$oextension "$fname"1500Frag.$oextension

#mp4fragment "$fname"800.$oextension "$fname"800Frag.$oextension

#mp4fragment "$fname"400.$oextension "$fname"400Frag.$oextension

mp4dash "$fname"1500Frag.$oextension "$fname"800Frag.$oextension "$fname"400Frag.$oextension -o "$fname"-DASH
result=$?
mv "$fname"pre.jpg "$fname"-DASH/pre.jpg
rm $fname.$fextension
rm $fpath/*.$oextension
exit $result
fi
