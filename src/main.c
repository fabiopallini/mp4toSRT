#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <vosk_api.h>
#include <sys/wait.h>
#include <parson.h>

void convertToSrt(const char *result, char *phrase, unsigned int *n_phrase, FILE *f);
void secondsToTime(char *arr, float sec);

int main(int argc, char *argv[]) {
	if(argc == 1) {
		printf("missing .mp4 filename\n");
		return 0;
	}
	int p[2];
	pid_t pid;
	char data[4000];

	VoskModel *model = vosk_model_new("model");
	VoskRecognizer *recognizer = vosk_recognizer_new(model, 16000.0);
	vosk_recognizer_set_words(recognizer, 1);
	int final;

	if (pipe(p)==-1)
		fprintf(stderr, "%s\n", "pipe error"); 

	if ((pid = fork()) == -1)
		fprintf(stderr, "%s\n", "fork error"); 

	if(pid == 0) {
		dup2(p[1], STDOUT_FILENO); 
		close(p[0]);
		close(p[1]);
		execl("ffmpeg", "ffmpeg", "-loglevel", "quiet", 
			"-i", argv[1], "-ar", "16000", "-ac", "1", "-f", "s16le", "-", NULL);
		perror("ffmpeg error:");
	} 
	else {
		close(p[1]);
		int nbytes; 
		//printf("Output: %s\n", data);
		char phrase[1000];
		unsigned int n_phrase = 0;
		FILE *f = fopen(strcat(argv[1], ".srt"), "w");

		while ((nbytes = read(p[0], data, sizeof(data))) != 0){
			final = vosk_recognizer_accept_waveform(recognizer, data, nbytes); 
			if (final) {
				const char *result = vosk_recognizer_result(recognizer);
				convertToSrt(result, phrase, &n_phrase, f);	
			}
		}

		//printf("RESULT: %s\n", vosk_recognizer_final_result(recognizer));
		const char *result = vosk_recognizer_final_result(recognizer);
		convertToSrt(result, phrase, &n_phrase, f);

		fclose(f);	
		vosk_recognizer_free(recognizer);
		vosk_model_free(model);
		wait(NULL);
	}

	return 0;
}

void convertToSrt(const char *result, char *phrase, unsigned int *n_phrase, FILE *f) {
	JSON_Value *json_root = json_parse_string(result);
	JSON_Object *json_object = json_value_get_object(json_root);
	printf("\n%s\n", json_object_get_string(json_object, "text"));

	JSON_Array *json_result = json_object_get_array(json_object, "result");
	unsigned int i = 0;
	unsigned k = 0;
	unsigned L = json_array_get_count(json_result);
	unsigned max_words = 7;
	if(L < max_words)
		max_words = L;
	for(i = 0; i < L; i++){
		JSON_Object *json_obj = json_array_get_object(json_result, i);
		strcat(phrase, json_object_get_string(json_obj, "word"));	
		strcat(phrase, " ");
		k += 1;
		if (k >= max_words || i >= L-1){
			float t1 = json_object_get_number(json_array_get_object(json_result, i-k+1), "start");
			float t2 = json_object_get_number(json_array_get_object(json_result, i), "end");
			char time_a[14];
			secondsToTime(time_a, t1);
			char time_b[14];
			secondsToTime(time_b, t2);
			fprintf(f, "%d\n%s --> %s\n%s\n\n", *n_phrase+=1,time_a, time_b, phrase);
			printf("%s --> %s \n%s\n\n", time_a, time_b, phrase);
			strcpy(phrase, "");
			k = 0;
		}
	}
	json_value_free(json_root);
}
void secondsToTime(char *arr, float sec) {
	int h = (sec/3600); 
	int m = (sec -(3600*h))/60;
	float s = (sec -(3600*h)-(m*60));
	
	char ss[7];
	if (s < 10)
		sprintf(ss, "0%.3f", s);
	else
		sprintf(ss, "%.3f", s);
	
	char mm[3];
	if (m < 10)
		sprintf(mm, "0%d", m);
	else
		sprintf(mm, "%d", m);
	
	char hh[3];
	if (h < 10)
		sprintf(hh, "0%d", h);
	else
		sprintf(hh, "%d", h);

	unsigned int i;			
	for(i = 0; i < 6; i++){
		if(ss[i] == '.'){
			ss[i] = ',';
			break;
		}
	}
	sprintf(arr, "%s:%s:%s", hh, mm, ss); 
}
