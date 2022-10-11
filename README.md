# mp4toSRT
Offline speech recognition and transcription to SRT file

### dependencies

1) You need to download at least one vosk model from [alphacephei](https://alphacephei.com/vosk/models)  
I suggest [vosk-model-en-us-0.22](https://alphacephei.com/vosk/models/vosk-model-en-us-0.22.zip) to maximise the result  
Once downloaded a vosk model, unzip the file and rename the folder to "model", then place it in the project's root.

2) You also may need ffmpeg if is not present in your system, place ffmpeg executable in /usr/bin/ (on Linux),  
otherwise you need to change the absolute path in the source code.

3) Vosk has been already compiled as dynamic library in his official [repository](https://github.com/alphacep/vosk-api/releases)  
download it and place libvosk.so under lib/ and you are ready to go!

### compile

```
> cd src
> make
```

### run

```
./mp4toSRT sample.mp4
```

the result will be in result.srt
